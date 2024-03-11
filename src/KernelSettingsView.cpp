/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <AppKit.h>
#include <InterfaceKit.h>
#include <StorageKit.h>
#include <Catalog.h>
#include <cstdio>
#include <private/interface/Spinner.h>
#include "KernelSettingsView.h"
#include "DataLoader.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Kernel settings"

class MyButton : public BButton
{
public:
    MyButton(const char* name,BMessage* msg = NULL, bool value = true)
    : BButton(name, NULL, msg, B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE),
      status(value ? B_CONTROL_ON : B_CONTROL_OFF)
    {
        SetBehavior(B_BUTTON_BEHAVIOR);

        size_t length;
        BResources *resource = BApplication::AppResources();

        const void* imagedata = resource->LoadResource(B_VECTOR_ICON_TYPE, "i_on", &length);
        if(imagedata) {
            icon_on = new BBitmap(BRect(0, 0, 16, 16), B_RGBA32);
            BIconUtils::GetVectorIcon((uint8*)imagedata, length, icon_on);
        }

        imagedata = resource->LoadResource(B_VECTOR_ICON_TYPE, "i_off", &length);
        if(imagedata) {
            icon_off = new BBitmap(BRect(0, 0, 16, 16), B_RGBA32);
            BIconUtils::GetVectorIcon((uint8*)imagedata, length, icon_off);
        }
        tt_on = B_TRANSLATE("Enabled. Click to disable and use automatic value.");
        tt_off = B_TRANSLATE("Disabled. Click to enable and use custom value.");
    }
    virtual void AttachedToWindow() {
        if(status == B_CONTROL_ON) {
            if(icon_on && icon_on->InitCheck() == B_OK)
                SetIcon(icon_on);
            SetToolTip(tt_on.String());
        }
        else {
            if(icon_off && icon_off->InitCheck() == B_OK)
                SetIcon(icon_off);
            SetToolTip(tt_off.String());
        }
    }
    virtual status_t Invoke(BMessage* msg = NULL) {
        if(status == B_CONTROL_ON) {
            status = B_CONTROL_OFF;
            if(icon_off && icon_off->InitCheck() == B_OK)
                SetIcon(icon_off);
            SetToolTip(tt_off.String());
        }
        else {
            status = B_CONTROL_ON;
            if(icon_on && icon_on->InitCheck() == B_OK)
                SetIcon(icon_on);
            SetToolTip(tt_on.String());
        }
        return B_OK;
    }
    void SetValue(bool value)
    {
        if(value) {
            status = B_CONTROL_ON;
            if(icon_on && icon_on->InitCheck() == B_OK)
                SetIcon(icon_on);
            SetToolTip(tt_on.String());
        }
        else {
            status = B_CONTROL_OFF;
            if(icon_off && icon_off->InitCheck() == B_OK)
                SetIcon(icon_off);
            SetToolTip(tt_off.String());
        }
    }
    bool Value() {
        if(status == B_CONTROL_ON)
            return true;
        else
            return false;
    }
private:
    BBitmap *icon_on;
    BBitmap *icon_off;
    BString  tt_on;
    BString  tt_off;
    int32    status;
};

struct spSettings {
    const char *label;
    uint32 message;
    int value;
} spData [] = {
    { B_TRANSLATE(  "9600 bit/s"), '9.6k',   9600 },
    { B_TRANSLATE( "19200 bit/s"), '19k ',  19200 },
    { B_TRANSLATE( "38400 bit/s"), '38k ',  38400 },
    { B_TRANSLATE( "57600 bit/s"), '57k ',  57600 },
    { B_TRANSLATE("115200 bit/s"), '115k', 115200 }
};

KernelSettingsView::KernelSettingsView(const char* title, std::vector<entry> entrylist)
: BView(BRect(), title, B_FOLLOW_ALL, B_ASYNCHRONOUS_CONTROLS),
  kernelsettings(entrylist)
{
    cbSMP = new BCheckBox(NULL, B_TRANSLATE("Disable multiprocessor support"), new BMessage('dsmp'));
    cbIOAPIC = new BCheckBox(NULL, B_TRANSLATE("Disable IO-APIC"), new BMessage('dioa'));
    cbAPM = new BCheckBox(NULL, B_TRANSLATE("APM support for legacy hardware"), new BMessage('eapm'));
    cbACPI = new BCheckBox(NULL, B_TRANSLATE("ACPI support for modern hardware"), new BMessage('acpi'));
    cb4GBLimit = new BCheckBox(NULL, B_TRANSLATE("Ignore all the memory beyond 4 GB"), new BMessage('4gbl'));
    cbfailsafevideo = new BCheckBox(NULL, B_TRANSLATE("Use fail safe video mode (VESA/framebuffer)"), new BMessage('fsvd'));

    btSMP = new MyButton("a", new BMessage('_smp'));
    btIOAPIC = new MyButton("b", new BMessage('_ioa'));
    btAPM = new MyButton("c", new BMessage('_apm'));
    btACPI = new MyButton("d", new BMessage('_acp'));
    bt4GBLimit = new MyButton("e", new BMessage('_4gb'));
    btfailsafevideo = new MyButton("f", new BMessage('_fsv'));

    BView *hardConfigView = new BView(NULL, B_SUPPORTS_LAYOUT, NULL);
    BLayoutBuilder::Group<>(hardConfigView, B_VERTICAL)
        .SetInsets(B_USE_WINDOW_INSETS)
        .SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE))
        .AddGrid(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, 1.0f)
            .Add(btSMP, 0, 0)
            .Add(cbSMP, 1, 0)
            .Add(btIOAPIC, 0, 1)
            .Add(cbIOAPIC, 1, 1)
            .Add(btAPM, 0, 2)
            .Add(cbAPM, 1, 2)
            .Add(btACPI, 0, 3)
            .Add(cbACPI, 1, 3)
            .Add(bt4GBLimit, 0, 4)
            .Add(cb4GBLimit, 1, 4)
            .Add(btfailsafevideo, 0, 5)
            .Add(cbfailsafevideo, 1, 5)
        .End()
        .AddGlue()
    .End();

    BPopUpMenu *speedMenu = new BPopUpMenu("");
    for(const auto& item : spData)
        speedMenu->AddItem(new BMenuItem(item.label, new BMessage(item.message)));
    speedMenu->SetTargetForItems(this);

    cbbsod = new BCheckBox(NULL, B_TRANSLATE("Enable Kernel Debugging Land"), new BMessage('bsod'));
    cbloadsymbols = new BCheckBox(NULL, B_TRANSLATE("Load kernel symbols"), new BMessage('symb'));
    cbemergkeys = new BCheckBox(NULL, B_TRANSLATE_COMMENT("Emergency keys (Alt+SysRq+*)",
        "Do not forget to translate the key names."), new BMessage('emrg'));
    cbserialdbgout = new BCheckBox(NULL, B_TRANSLATE("Serial debug output"), new BMessage('dbgo'));
    sbbochsdbgout = new BCheckBox(NULL, B_TRANSLATE("Bochs debug output"), new BMessage('boch'));
    cblakplink = new BCheckBox(NULL, B_TRANSLATE("Parallel debug output"), new BMessage('para'));
    iserialdbgport = new BTextControl(B_TRANSLATE("Serial debug port"), "", new BMessage('port'));
    iserialdbgspd = new BMenuField("mf_dbgspeed", B_TRANSLATE("Serial debug speed"), speedMenu);

    btbsod = new MyButton("g", new BMessage('_kdl'));
    btloadsymbols = new MyButton("h", new BMessage('_sym'));
    btemergkeys = new MyButton("i", new BMessage('_emr'));
    btserialdbgout = new MyButton("j", new BMessage('_sdb'));
    btbochsdbgout = new MyButton("k", new BMessage('_bch'));
    btlakplink = new MyButton("l", new BMessage('_lak'));
    btserialdbgport = new MyButton("m", new BMessage('_dbp'));
    btserialdbgspd = new MyButton("n", new BMessage('_dbs'));

    BView *debugConfigView = new BView(NULL, B_SUPPORTS_LAYOUT, NULL);
    BLayoutBuilder::Group<>(debugConfigView, B_VERTICAL)
        .SetInsets( B_USE_WINDOW_INSETS)
        .SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE))
        .AddGrid(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, 1.0f)
            .Add(btbsod, 0, 0)
            .Add(cbbsod, 1, 0)
            .Add(btloadsymbols, 0, 1)
            .Add(cbloadsymbols, 1, 1)
            .Add(btemergkeys, 0, 2)
            .Add(cbemergkeys, 1, 2)
            .Add(btserialdbgout, 0, 3)
            .Add(cbserialdbgout, 1, 3)
            .Add(btserialdbgport, 0, 4)
            .Add(iserialdbgport, 1, 4)
            .Add(btserialdbgspd, 0, 5)
            .Add(iserialdbgspd, 1, 5)
            .Add(btbochsdbgout, 0, 6)
            .Add(sbbochsdbgout, 1, 6)
            .Add(btlakplink, 0, 7)
            .Add(cblakplink, 1, 7)
        .End()
        .AddGlue()
    .End();

    cbsyslogout = new BCheckBox(NULL, B_TRANSLATE("Syslog: debug output"), new BMessage('slou'));
    cbsyslogtime = new BCheckBox(NULL, B_TRANSLATE("Syslog: include time stamps"), new BMessage('timl'));
    isyslogbufsz = new BSpinner("name", B_TRANSLATE("Syslog: buffer size"), new BMessage('slbz'));
    isyslogmaxsz  = new BSpinner("name", B_TRANSLATE("Syslog: max size"), new BMessage('symx'));

    btsyslogout = new MyButton("o", new BMessage('_log'));
    btsyslogtime = new MyButton("p", new BMessage('_lgt'));
    btsyslogbufsz = new MyButton("q", new BMessage('_lgb'));
    btsyslogmaxsz = new MyButton("r", new BMessage('_lgx'));

    BView *loggingConfigView = new BView(NULL, B_SUPPORTS_LAYOUT, NULL);
    BLayoutBuilder::Group<>(loggingConfigView, B_VERTICAL)
        .SetInsets( B_USE_WINDOW_INSETS)
        .SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE))
        .AddGrid(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, 1.0f)
            .Add(btsyslogout, 0, 0)
            .Add(cbsyslogout, 1, 0)
            .Add(btsyslogtime, 0, 1)
            .Add(cbsyslogtime, 1, 1)
            .Add(btsyslogbufsz, 0, 2)
            .Add(isyslogbufsz, 1, 2)
            .Add(btsyslogmaxsz, 0, 3)
            .Add(isyslogmaxsz, 1, 3)
        .End()
        .AddGlue()
    .End();

    cbqemusingle = new BCheckBox(NULL, B_TRANSLATE("Enable single step hack for QEMU"), new BMessage('qemu'));

    btqemusingle = new MyButton("s", new BMessage('_qmu'));

    BView *miscConfigView = new BView(NULL, B_SUPPORTS_LAYOUT, NULL);
    BLayoutBuilder::Group<>(miscConfigView, B_VERTICAL)
        .SetInsets( B_USE_WINDOW_INSETS)
        .SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE))
        .AddGrid(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, 1.0f)
            .Add(btqemusingle, 0, 0)
            .Add(cbqemusingle, 1, 0)
        .End()
        .AddGlue()
    .End();

    _Init();

    fListView = new BListView("cat_list");
    fListView->SetSelectionMessage(new BMessage(KS_ITEM_SELECTED));
    fListView->AddItem(new BStringItem(B_TRANSLATE("Hardware configuration")));
    fListView->AddItem(new BStringItem(B_TRANSLATE("Debugging")));
    fListView->AddItem(new BStringItem(B_TRANSLATE("Logging")));
    fListView->AddItem(new BStringItem(B_TRANSLATE("Miscellaneous")));
    BScrollView *listScrollView = new BScrollView("sc_cont", fListView,
        0, false, true, B_FANCY_BORDER);
    fListView->SetExplicitMinSize(
        BSize(fListView->StringWidth(B_TRANSLATE("Hardware configuration")) + 10,
        B_SIZE_UNSET));

    refreshButton = new BButton(B_TRANSLATE("Update"), new BMessage('updt'));
    refreshButton->SetExplicitSize(BSize(listScrollView->MinSize().Width(), B_SIZE_UNSET));
    resetButton = new BButton(B_TRANSLATE("Restore"), new BMessage('rstr'));
    resetButton->SetExplicitSize(BSize(listScrollView->MinSize().Width(), B_SIZE_UNSET));

    fCardView = new BCardView();
    fCardView->AddChild(hardConfigView);
    fCardView->AddChild(debugConfigView);
    fCardView->AddChild(loggingConfigView);
    fCardView->AddChild(miscConfigView);

    BLayoutBuilder::Group<>(this, B_HORIZONTAL, 10)
        .SetInsets(B_USE_WINDOW_SPACING)
        .AddGroup(B_VERTICAL)
            .Add(listScrollView)
            .Add(refreshButton)
            .Add(resetButton)
        .End()
        .Add(fCardView)
        .AddGlue()
    .End();
}

void KernelSettingsView::_Init()
{
    BString value;
    bool isenabled;

    GetValueForKey("disable_smp", &value, &isenabled);
    cbSMP->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)btSMP)->SetValue(isenabled);

    GetValueForKey("disable_ioapic", &value, &isenabled);
    cbIOAPIC->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)btIOAPIC)->SetValue(isenabled);

    GetValueForKey("apm", &value, &isenabled);
    cbAPM->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)btAPM)->SetValue(isenabled);

    GetValueForKey("acpi", &value, &isenabled);
    cbACPI->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)btACPI)->SetValue(isenabled);

    GetValueForKey("4gb_memory_limit", &value, &isenabled);
    cb4GBLimit->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)bt4GBLimit)->SetValue(isenabled);

    GetValueForKey("fail_safe_video_mode", &value, &isenabled);
    cbfailsafevideo->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)btfailsafevideo)->SetValue(isenabled);

    GetValueForKey("bluescreen", &value, &isenabled);
    cbbsod->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)btbsod)->SetValue(isenabled);

    GetValueForKey("load_symbols", &value, &isenabled);
    cbloadsymbols->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)btloadsymbols)->SetValue(isenabled);

    GetValueForKey("emergency_keys", &value, &isenabled);
    cbemergkeys->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)btemergkeys)->SetValue(isenabled);

    GetValueForKey("serial_debug_output", &value, &isenabled);
    cbserialdbgout->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)btserialdbgout)->SetValue(isenabled);

    GetValueForKey("serial_debug_port", &value, &isenabled);
    iserialdbgport->SetText(value.String());
    ((MyButton*)btserialdbgport)->SetValue(isenabled);

    GetValueForKey("serial_debug_speed", &value, &isenabled);
    int count = iserialdbgspd->Menu()->CountItems();
    int i = 0;
    while(i < count && iserialdbgspd->Menu()->ItemAt(i) &&
    strstr(iserialdbgspd->Menu()->ItemAt(i)->Label(), value) == nullptr)
        i++;
    iserialdbgspd->Menu()->ItemAt(i)->SetMarked(true);
    ((MyButton*)btserialdbgspd)->SetValue(isenabled);

    GetValueForKey("bochs_debug_output", &value, &isenabled);
    sbbochsdbgout->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)btbochsdbgout)->SetValue(isenabled);

    GetValueForKey("laplinkll_debug_output", &value, &isenabled);
    cblakplink->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)btlakplink)->SetValue(isenabled);

    GetValueForKey("syslog_debug_output", &value, &isenabled);
    cbsyslogout->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)btsyslogout)->SetValue(isenabled);

    GetValueForKey("syslog_time_stamps", &value, &isenabled);
    cbsyslogtime->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)btsyslogtime)->SetValue(isenabled);

    GetValueForKey("syslog_buffer_size", &value, &isenabled);
    isyslogbufsz->SetValue(atoi(value));
    ((MyButton*)btsyslogbufsz)->SetValue(isenabled);

    GetValueForKey("syslog_max_size", &value, &isenabled);
    isyslogmaxsz->SetValue(atoi(value));
    ((MyButton*)btsyslogmaxsz)->SetValue(isenabled);

    GetValueForKey("qemu_single_step_hack", &value, &isenabled);
    cbqemusingle->SetValue(value == "true" ? B_CONTROL_ON : B_CONTROL_OFF);
    ((MyButton*)btqemusingle)->SetValue(isenabled);
}

void KernelSettingsView::_Update()
{
    kernelsettings.clear();
    kernelsettings_load(&kernelsettings);
    _Init();
}

void KernelSettingsView::AttachedToWindow()
{
    fListView->SetTarget(this);
    // fCardView->SetTarget(this);
    refreshButton->SetTarget(this);
    resetButton->SetTarget(this);

    cbSMP->SetTarget(this);
    cbIOAPIC->SetTarget(this);
    cbAPM->SetTarget(this);
    cbACPI->SetTarget(this);
    cb4GBLimit->SetTarget(this);
    cbfailsafevideo->SetTarget(this);

    cbbsod->SetTarget(this);
    cbloadsymbols->SetTarget(this);
    cbemergkeys->SetTarget(this);
    cbserialdbgout->SetTarget(this);
    iserialdbgport->SetTarget(this);
    sbbochsdbgout->SetTarget(this);
    cblakplink->SetTarget(this);

    cbsyslogout->SetTarget(this);
    cbsyslogtime->SetTarget(this);
    isyslogbufsz->SetTarget(this);
    isyslogmaxsz->SetTarget(this);

    cbqemusingle->SetTarget(this);
}

void KernelSettingsView::MessageReceived(BMessage* msg)
{
    switch(msg->what)
    {
        case KS_ITEM_SELECTED:
        {
            int32 index = msg->GetInt32("index", 0);
            if (index >= 0)
				fCardView->CardLayout()->SetVisibleItem(index);
            break;
        }
        case '_smp':
        case 'dsmp':
        {
            SaveKey("disable_smp",
                cbSMP->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)btSMP)->Value());
            _Update();
            break;
        }
        case '_ioa':
        case 'dioa':
        {
            SaveKey("disable_ioapic",
                cbIOAPIC->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)btIOAPIC)->Value());
            _Update();
            break;
        }
        case '_apm':
        case 'eapm':
        {
            SaveKey("apm",
                cbAPM->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)btAPM)->Value());
            _Update();
            break;
        }
        case '_acp':
        case 'acpi':
        {
            SaveKey("acpi",
                cbACPI->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)btACPI)->Value());
            _Update();
            break;
        }
        case '_4gb':
        case '4gbl':
        {
            SaveKey("4gb_memory_limit",
                cb4GBLimit->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)bt4GBLimit)->Value());
            _Update();
            break;
        }
        case '_fsv':
        case 'fsvd':
        {
            SaveKey("fail_safe_video_mode",
                cbfailsafevideo->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)btfailsafevideo)->Value());
            _Update();
            break;
        }
        case '9.6k':
        case '19k ':
        case '38k ':
        case '57k ':
        case '115k':
        {
            int i = 0;
            int j = sizeof(spData)/sizeof(spData[0]);
            while(i < j && spData[i].message != msg->what)
                i++;
            SaveKey("serial_debug_speed", std::to_string(spData[i].value).c_str(),
                ((MyButton*)btserialdbgspd)->Value());
            _Update();
            break;
        }
        case 'port':
        {
            SaveKey("serial_debug_port", iserialdbgport->Text(),
                ((MyButton*)btserialdbgport)->Value());
            _Update();
            break;
        }
        case '_kdl':
        case 'bsod':
        {
            SaveKey("bluescreen",
                cbbsod->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)btbsod)->Value());
            _Update();
            break;
        }
        case '_sym':
        case 'symb':
        {
            SaveKey("load_symbols",
                cbloadsymbols->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)btloadsymbols)->Value());
            _Update();
            break;
        }
        case '_emr':
        case 'emrg':
        {
            SaveKey("emergency_keys",
                cbemergkeys->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)btemergkeys)->Value());
            _Update();
            break;
        }
        case '_sdb':
        case 'dbgo':
        {
            SaveKey("serial_debug_output",
                cbserialdbgout->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)btserialdbgout)->Value());
            _Update();
            break;
        }
        case '_bch':
        case 'boch':
        {
            SaveKey("bochs_debug_output",
                sbbochsdbgout->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)btbochsdbgout)->Value());
            _Update();
            break;
        }
        case '_lak':
        case 'para':
        {
            SaveKey("laplinkll_debug_output",
                cblakplink->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)btlakplink)->Value());
            _Update();
            break;
        }
        case '_log':
        case 'slou':
        {
            status_t status = SaveKey("serial_debug_output",
                cbsyslogout->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)btsyslogout)->Value());
            _Update();
            break;
        }
        case '_lgt':
        case 'timl':
        {
            SaveKey("syslog_time_stamps",
                cbSMP->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)cbsyslogtime)->Value());
            _Update();
            break;
        }
        case '_lgb':
        case 'slbz':
        {

            _Update();
            break;
        }
        case '_lgx':
        case 'symx':
        {

            _Update();
            break;
        }
        case '_qmu':
        case 'qemu':
        {
            SaveKey("qemu_single_step_hack",
                cbqemusingle->Value() == B_CONTROL_ON ? "true" : "false",
                ((MyButton*)btqemusingle)->Value());
            _Update();
            break;
        }
        case 'updt':
            _Update();
            break;
        case 'rstr':
        {
            BAlert *alert = new BAlert(B_TRANSLATE_COMMENT("Restore default", "This will not be visible"),
                B_TRANSLATE("Do you want to restore kernel settings default values?"),
                B_TRANSLATE("Do not restore"), B_TRANSLATE("Restore"));
            alert->SetShortcut(0, B_ESCAPE);
            int32 result = alert->Go();
            if(result == 1) {
                kernelsettings.clear();
                kernelsettings_default(&kernelsettings);
                kernelsettings_save(kernelsettings);
                kernelsettings_load(&kernelsettings);
                _Init();
            }
            break;
        }
        default:
            // fprintf(stderr, "Message: %u. Not processed.\n", msg->what);
            BView::MessageReceived(msg);
            break;
    }
}


status_t KernelSettingsView::GetValueForKey(BString key, BString* value, bool* on)
{
    for(auto item = kernelsettings.begin(); item != kernelsettings.end(); ++item) {
        if(item->key == key) {
            *value = item->value;
            *on = item->enabled;
            return B_OK;
        }
    }
    return B_ERROR;
}

status_t KernelSettingsView::SaveKey(BString key, BString value, bool enabled)
{
    status_t status;

    status = kernelsettings_entry_edit(&kernelsettings, key, value, enabled);
    if(status != B_OK) {
        fprintf(stderr, "ERROR EDITING!!!!\n");
        return status;
    }
    else
        fprintf(stderr, "SUCCESS EDITING!!!!\n");

    for(const auto& item : kernelsettings)
        fprintf(stderr, "Retrieved: key(\"%s\"):value(\"%s\"). Enabled? %s.\n",
            item.key.String(), item.value.String(), item.enabled ? "true" : "false");

    status = kernelsettings_save(kernelsettings);
    if(status != B_OK) {
        fprintf(stderr, "ERROR SAVING!!!!\n");
        return status;
    }
    else
        fprintf(stderr, "SUCCESS SAVING!!!!\n");

    return status;
}

#include <Catalog.h>
#include <cstdio>
#include "TextEditorView.h"
#include "DataLoader.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "text editor view"

TextEditorView::TextEditorView(const char* title, BString data, const char* path)
: BView(BRect(), title, B_FOLLOW_ALL, B_ASYNCHRONOUS_CONTROLS),
  indata(data), targetfile(path), readonly(false)
{
    ckreadonly = new BCheckBox("cb_editable", B_TRANSLATE("Read only"), new BMessage(TEV_RDONLY));
    ckreadonly->SetValue(B_CONTROL_OFF);

    saveButton = new BButton("bt_save", B_TRANSLATE("Save"), new BMessage(TEV_SAVE));
    updateButton = new BButton("bt_update", B_TRANSLATE("Update"), new BMessage(TEV_UPDATE));
    resetButton  = new BButton("bt_reset", B_TRANSLATE("Reset"), new BMessage(TEV_RESTORE));
    openextButton = new BButton("bt_penext", B_TRANSLATE("Open in editor"), new BMessage(TEV_OPENEXT));

    BFont font(be_fixed_font);
    font.SetFace(B_BOLD_FACE);
    const rgb_color black = {0,0,0};
    textview = new BTextView("", &font, &black, B_WILL_DRAW | B_FRAME_EVENTS | B_PULSE_NEEDED | B_INPUT_METHOD_AWARE);
    textview->MakeEditable(!readonly);
    BScrollView *sctv = new BScrollView("sctextview", textview, B_FRAME_EVENTS, true, true, B_FANCY_BORDER);

    _Init();

    BLayoutBuilder::Group<>(this, B_HORIZONTAL)
        .Add(sctv)
        .AddGroup(B_VERTICAL)
            .SetInsets(0, 0, B_USE_DEFAULT_SPACING, 0)
            .Add(saveButton)
            .Add(updateButton)
            .AddStrut(10.0f)
            .Add(resetButton)
            .AddGlue()
            .Add(ckreadonly)
            .Add(openextButton)
        .End()
    .End();
}

TextEditorView::~TextEditorView()
{
}

void TextEditorView::_Init()
{
    textview->SetText(indata.String());
}

void TextEditorView::_Update()
{
    userscript_load(targetfile, &indata);
    _Init();
}

void TextEditorView::AttachedToWindow()
{
    ckreadonly->SetTarget(this);
    saveButton->SetTarget(this);
    updateButton->SetTarget(this);
    resetButton->SetTarget(this);
    openextButton->SetTarget(this);

    float width, maxwidth, height;
    saveButton->GetPreferredSize(&width, &height);
    maxwidth = width;
    updateButton->GetPreferredSize(&width, &height);
    if(width > maxwidth)
    	maxwidth = width;
    resetButton->GetPreferredSize(&width, &height);
    if(width > maxwidth)
    	maxwidth = width;
    openextButton->GetPreferredSize(&width, &height);
    if(width > maxwidth)
    	maxwidth = width;

    saveButton->SetExplicitSize(BSize(maxwidth, height));
    updateButton->SetExplicitSize(BSize(maxwidth, height));
    resetButton->SetExplicitSize(BSize(maxwidth, height));
	openextButton->SetExplicitSize(BSize(maxwidth, height));

    BView::AttachedToWindow();
}

void TextEditorView::MessageReceived(BMessage* msg)
{
    switch(msg->what)
    {
        case TEV_RDONLY:
            if(ckreadonly->Value() == B_CONTROL_ON)
                readonly = true;
            else if(ckreadonly->Value() == B_CONTROL_OFF)
                readonly = false;
            textview->MakeEditable(!readonly);
            break;
        case TEV_SAVE:
            if(userscript_save(targetfile, BString(textview->Text())) != B_OK)
                fprintf(stderr, "Error: file %s could not be saved.\n", targetfile);
            _Update();
            break;
        case TEV_UPDATE:
            _Update();
            break;
        case TEV_RESTORE:
        {
            BAlert *alert = new BAlert(B_TRANSLATE_COMMENT("Restore?", "This will not be visible"),
                B_TRANSLATE("Restore to default? The current file's contents will be replaced."),
                B_TRANSLATE("Do not restore"), B_TRANSLATE("Restore"), NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
            alert->SetShortcut(0, B_ESCAPE);
            int32 result = alert->Go();
            if(result == 0)
                break;

            BPath fullpath(targetfile);
            userscript_default(fullpath.Leaf(), &indata);
            textview->Delete();
            textview->SetText(indata.String());
            userscript_save(targetfile, BString(textview->Text()));
            _Update();
            break;
        }
        case TEV_OPENEXT:
        {
            if(!exists(targetfile)) {
                BAlert *alert = new BAlert(B_TRANSLATE_COMMENT("Entry missing", "This will not be visible"),
                        B_TRANSLATE("This file currently does not exist. Do you want to create it?"),
                        B_TRANSLATE("Do not create"), B_TRANSLATE("Create"));
                alert->SetShortcut(0, B_ESCAPE);
                int32 result = alert->Go();
                if(result == 0)
                    break;
                status_t status = userscript_create(targetfile);
                if(status != B_OK)
                    fprintf(stderr, "Error: file could not be created.\n");
            }
            launch("text/x-source-code", targetfile);
            break;
        }
        default:
            BView::MessageReceived(msg);
            break;
    }
}


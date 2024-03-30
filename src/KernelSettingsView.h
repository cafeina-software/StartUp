#ifndef __KERNEL_SETTINGS_H
#define __KERNEL_SETTINGS_H

#include "DataLoader.h"
#include <InterfaceKit.h>
#include <map>
#include <private/interface/AbstractSpinner.h>
#include <private/interface/Spinner.h>
#include <string>
#include <unordered_map>

#define KS_ITEM_SELECTED  'ksis'
#define KS_ITEM_CHANGED   'ksic'
#define KS_OPEN_EXT       'kext'

class KernelSettingsView : public BView {
public:
                        KernelSettingsView(const char* title, std::vector<entry> entrylist);
    virtual void        AttachedToWindow  ();
    virtual void        MessageReceived   (BMessage* msg);
    status_t            GetValueForKey    (BString key, BString*, bool*);
    status_t            SaveKey           (BString key, BString value, bool enabled);
    void                Update            ();
    void                RestoreDefault    ();
private:
    void                _Init             ();
private:
    BListView    *fListView;
    BCardView    *fCardView;
    BCheckBox    *cbSMP,
                 *cbIOAPIC,
                 *cbAPM,
                 *cbACPI,
                 *cb4GBLimit,
                 *cbfailsafevideo,
                 *cbbsod,
                 *cbloadsymbols,
                 *cbemergkeys,
                 *cbserialdbgout,
                 *cbsyslogout,
                 *cbsyslogtime,
                 *sbbochsdbgout,
                 *cblakplink,
                 *cbqemusingle;
    BTextControl *iserialdbgport;
    BSpinner     *isyslogbufsz,
                 *isyslogmaxsz;
    BMenuField   *iserialdbgspd;

    BButton      *btSMP,
                 *btIOAPIC,
                 *btAPM,
                 *btACPI,
                 *bt4GBLimit,
                 *btfailsafevideo,
                 *btbsod,
                 *btloadsymbols,
                 *btemergkeys,
                 *btserialdbgout,
                 *btsyslogout,
                 *btsyslogtime,
                 *btbochsdbgout,
                 *btlakplink,
                 *btqemusingle,
                 *btserialdbgport,
                 *btsyslogbufsz,
                 *btsyslogmaxsz,
                 *btserialdbgspd;
    BButton      *refreshButton,
                 *resetButton,
                 *openextButton;
    std::vector<entry> kernelsettings;
    // std::unordered_map<BString, bool> togglemap;
};

#endif /* __KERNEL_SETTINGS_H */

#ifndef __TEXT_EDITOR_VIEW_H
#define __TEXT_EDITOR_VIEW_H

#include <AppKit.h>
#include <InterfaceKit.h>

#define TEV_SAVE    'save'
#define TEV_UPDATE  'load'
#define TEV_RESTORE 'rest'
#define TEV_OPENEXT 'extr'
#define TEV_RDONLY  'rdon'

class TextEditorView : public BView
{
public:
        TextEditorView      (const char* tabtitle, BString data, const char* what);
    virtual
        ~TextEditorView     ();
    virtual void
        MessageReceived     (BMessage*);
    virtual void
        AttachedToWindow    ();
    void _Init();
    void _Update();
private:
    BString     indata;
    const char *targetfile;
    bool        readonly;
    BButton    *saveButton,
               *updateButton,
               *resetButton,
               *openextButton;
    BCheckBox  *ckreadonly;
    BTextView  *textview;
};

#endif /* __TEXT_EDITOR_VIEW_H */

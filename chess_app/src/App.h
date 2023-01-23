#pragma once

#include <wx/wx.h>
#include "Main.h"
#include "../../chessboard/chessboard.h"

class App : public wxApp
{
public:
    App();
    ~App();

private:
    Main* m_frame1 = nullptr;

public:
    virtual bool OnInit();
};
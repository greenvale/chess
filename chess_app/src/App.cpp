#include "App.h"
#include "Main.h"

IMPLEMENT_APP(App); // main fcn

App::App()
{

}

App::~App()
{

}

bool App::OnInit()
{
    m_frame1 = new Main();
    m_frame1->Show();

    return true;
}
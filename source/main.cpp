// David Hart 2011

#include "GXBase.h"
#include "MyWindow.h"

class MyApp :public gxbase::App
{
	
public:
	
	MyApp();

private:

	MyWindow w;
	
};

MyApp::MyApp()
{
	w.SetGXApp(this);
}

static MyApp tut1;

#include <Fl/Fl.H>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Window.H>
#include <Fl/Fl_Input.H>
#include <Fl/Fl_Text_Display.H>

int main(){
    Fl_Window *window=new Fl_Window(400,300,"ChatTCP");
    Fl_Text_Display *txt_area=new Fl_Text_Display(10,10,300,200);
    Fl_Input *txtinput=new Fl_Input(10,200,300,30);
    Fl_Button *button=new Fl_Button(300,200,70,30,"Send");
    window->end();
    window->show();
    return Fl::run();
}
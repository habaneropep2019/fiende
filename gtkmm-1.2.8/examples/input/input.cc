#include <gtk--/main.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <memory> 

using std::istream;

using std::auto_ptr;

using SigC::slot;

auto_ptr<istream> input;


// this will be our callback for read operations
// there is not much to say. just read a string,
// print it and quit the application if the string was quit
void MyCallback(int, GdkInputCondition) {
  Gtk::string dummy;
  do {
    (*input) >> dummy;
    cout << dummy << endl;
    if(dummy == "quit") Gtk::Main::quit();
  } while(input->fail());
}


int main (int argc, char *argv[])
{
  // the usual Gtk::Main object
  Gtk::Main app(argc, argv);

  // create a fifo for testing purposes
  if (mkfifo("testfifo",0666) != 0) { 
    cerr << "error creating fifo" << endl;
    return -1;
  }
  
  // open the fifo
  input=new ifstream("testfifo");
  
//    int fd = open("testfifo", 0);
//    if (fd == -1) {
//      cerr << "error opening fifo" << endl;
//      return -1;
//    }

  // assign the fifo's filedescriptor to our ifstream object
  //This sucks; it will only ever work with libstdc++-v3, as
  //  both istream::__filebuf_type and the basic_filebuf contructor
  //  that takes an fd are libstdc++-v3 specific.
  //input=new istream(new ifstream::__filebuf_type(fd,"testfifo"));
  
  // connect the callback function
  app.input.connect(slot(MyCallback), fd, GDK_INPUT_READ);

  // and last but not least - run the application main loop
  app.run();

  // now remove the temporary fifo
  if(unlink("testfifo")) 
    cerr << "error removing fifo" << endl;

  return 0;
}

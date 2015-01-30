#include <iostream>
#include <string>
#include <iomanip>
#include "sprite.h"
#include "gamedata.h"
#include "manager.h"

Manager::~Manager() { 
  // Manager made it, so Manager needs to delete it
  SDL_FreeSurface(orbSurface);
  SDL_FreeSurface(backSurface);
  delete orbFrame;
  delete backFrame;
  //delete IOManager::getInstance();
  //delete Gamedata::getInstance();
}

Manager::Manager() :
  env( SDL_putenv(const_cast<char*>("SDL_VIDEO_CENTERED=center")) ),
  io( IOManager::getInstance() ),
  clock( Clock::getInstance() ),
  screen( io.getScreen() ),
  backRed( Gamedata::getInstance().getXmlInt("back/red") ),
  backGreen( Gamedata::getInstance().getXmlInt("back/green") ),
  backBlue( Gamedata::getInstance().getXmlInt("back/blue") ),

  //orb("greenorb", orbFrame),
  orbSurface( io.loadAndSet(
    Gamedata::getInstance().getXmlStr("greenorb/file"), 
    Gamedata::getInstance().getXmlBool("greenorb/transparency")) ),
  orbFrame( new Frame("greenorb", orbSurface) ),

  backSurface( io.loadAndSet(
    Gamedata::getInstance().getXmlStr("back/file"), 
    Gamedata::getInstance().getXmlBool("back/transparency")) ),
  backFrame( new Frame("back", backSurface) ),
  orbs(),

  makeVideo( false ),
  frameCount( 0 ),
  username(  Gamedata::getInstance().getXmlStr("username") ),
  frameMax( Gamedata::getInstance().getXmlInt("frameMax") ),
  TITLE( Gamedata::getInstance().getXmlStr("screenTitle") )
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    throw string("Unable to initialize SDL: ");
  }

  for ( int i=0; i<Gamedata::getInstance().getXmlInt("greenorb/number");++i){
  	orbs.push_back(Sprite("greenorb",orbFrame));
  }
  atexit(SDL_Quit);
  /*
  for (unsigned int i=0; i < 50;++i){
    orbs.push_back(Sprite("greenorb",orbFrame));
  }*/
}

void Manager::drawBackground() const {
  SDL_FillRect( screen, NULL, 
    SDL_MapRGB(screen->format, backRed, backGreen, backBlue) );
  SDL_Rect dest = {0, 0, 0, 0};
  SDL_BlitSurface( screen, NULL, screen, &dest );
 // SDL_BlitSurface( backSurface, NULL, screen, &dest );
}

void Manager::draw() const {
 backFrame->draw(0,0);
  //orb.draw();
  for(unsigned int i=0;i<orbs.size();++i){
    orbs[i].draw();
  }
  io.printMessageCenteredAt(TITLE, 10);
  io.printMessageValueAt("fps: ", clock.getFps(), 10, 10);
  io.printMessageValueAt("tick: ", clock.getTicks(), 10, 30);
  SDL_Flip(screen);
}

void Manager::update() {
  ++clock;
  Uint32 ticks = clock.getElapsedTicks();
  //orb.update(ticks);
  for(unsigned int i=0;i<orbs.size();++i){
    orbs[i].update(ticks);
  }

  if ( makeVideo && frameCount < frameMax ) {
    std::stringstream strm;
    strm << "frames/" << username<< '.' 
         << std::setfill('0') << std::setw(4) 
         << frameCount++ << ".bmp";
    std::string filename( strm.str() );
    std::cout << "Making frame: " << filename << std::endl;
    SDL_SaveBMP(screen, filename.c_str());
  }
}

void Manager::play() {
  SDL_Event event;

  bool done = false;
  bool keyCatch = false;
  while ( not done ) {

    SDL_PollEvent(&event);
    Uint8 *keystate = SDL_GetKeyState(NULL);
    if (event.type ==  SDL_QUIT) { done = true; break; }
    if(event.type == SDL_KEYUP) { 
      keyCatch = false; 
    }
    if(event.type == SDL_KEYDOWN) {
      if (keystate[SDLK_ESCAPE] || keystate[SDLK_q]) {
        done = true;
        break;
      }

      if (keystate[SDLK_F4] && !makeVideo) {
        std::cout << "Making video frames" << std::endl;
        makeVideo = true;
      }
      if (keystate[SDLK_p] && !keyCatch) {
        keyCatch = true;
        if ( clock.isPaused() ) clock.unpause();
        else clock.pause();
      }
    }

    draw();
    update();
  }
}

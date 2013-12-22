# Avatar Robot

## Directory

### android/

* android app to control the micon board and to stream the sound.

### client/

* User interface for the Web browser.

### server/wserver.rb

* relays control signal

### server/Kanjikoe/

* converts the Japanese string to the voice chip's command.

### physical/board

* PSoC micon program for machine control.

### physical/acryl.ai

* Acril cutting data.

## Data flow

Data flow of the control signal and machine status.

* control signal

        WEB browser (client/) -> server (server/) -> Android -> app (android/) -> micon (physical/board) -> machine

* machine status

        machine -> micon (physical/board) -> app (android/) -> Android -> server (server/wserver.rb) -> WEB browser (client/)

Data flow of the Captured image and sound.

* image

        android -> vendor app (https://play.google.com/store/apps/details?id=com.pas.webcam) -> server (Routing) -> web browser (client/)

* sound

        android -> app (android/) -> server (server/wserver.rb) -> web browser (client/)

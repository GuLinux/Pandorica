# Pandorica #
Simple yet powerful Web Media Player

This HTML5-based Media Player will allow you to stream your multimedia collection in a local network, or over the web.  
It's written in C++ (C++11,  to be precise), using [Wt](http://www.webtoolkit.eu/wt), [Twitter Bootstrap](http://twitter.github.io/bootstrap) and [MediaElement.js](http://mediaelementjs.com/)

## Dependencies ##
* [Wt](http://www.webtoolkit.eu/wt) 3.3.0 or above
* [Boost](http://boost.org)
* [GraphicsMagick++](http://www.graphicsmagick.org/Magick++/) ([ImageMagick++](http://www.imagemagick.org/script/index.php) should work too, but I found it much less reliable)
* [FFmpeg binary,  libavformat/libavcodec/libavutil](http://www.ffmpeg.org)
* [FFmpeg Thumbnailer](https://code.google.com/p/ffmpegthumbnailer/)
* [Qt4](http://qt-project.org/) (Optional, to add a system tray icon when your server is running)

Also make sure to have a recent gcc version supporting C++11.

### Database ###
At least one of
* [Sqlite3](http://www.sqlite.org/)
* [PostgreSQL](http://www.postgresql.org/)
Mysql/MariaDB will probably be supported soon.

## Quick Start ##
First, make sure to have all dependencies installed (if you are compiling from source code, you need the headers packages too). 

Then, clone the repository and update the submodules. 

    $ git clone https://github.com/rockman81/Pandorica.git
    $ cd Pandorica/
    $ git submodule init
    $ git submodule update
    
Configure and build *Pandorica* 

    $ mkdir build
    $ cd build
    $ cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr
    $ make
    $ sudo make install
    
Then, just run the '*Pandorica*' command and open your browser to http://localhost:8080.

## Advanced configuration and customization ##
Use the --help option for the most common options,  and the --help-full option to see all command line options (including Wt default ones). 
Lookup the official [Wt Reference Page](http://www.webtoolkit.eu:3080/wt/doc/reference/html/overview.html#wthttpd) for a detailed overview.

*Pandorica* also uses some extra configuration properties. 
You can set them in the "properties" section of your *wt_config.xml* file (usually located in /etc/wt, you can use a different one with the "-c" option).

### PostgreSQL connection parameters ###
Example property entry:

    <property name="psql-connection">application_name=streamingapp host=localhost port=5432 dbname=streaming_app_database user=streamingapp_pg_username password=streamingapp_pg_password</property>
    
If *psql-connection* property is set, *Pandorica* will **not** use the Sqlite3 connector.

### Authentication and Email settings ###

Email verification (strongly suggested if you are deploying your *Pandorica* box on the internet):

    <property name="email-verification-mandatory">true</property>
    
Email addresses:

    <!-- Streaming App settings -->
    <property name="auth-mail-sender-name">Streaming App</property>
    <property name="auth-mail-sender-address">admin@localhost</property>
    <!-- put a REAL address here, you will have to receive warnings here! -->
    <property name="admin-mail-name">StreamingApp Admin</message>
    <property name="admin-mail-address">admin@localhost</message>

Remember to install a local SMTP server to relay emails, particularly if you set email-verification-mandatory true. 
If you don't,  your users will not be able to login!
    
You can also enable OAuth authentication: [Google](http://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1Auth_1_1GoogleService.html#details) and [Facebook](http://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1Auth_1_1FacebookService.html#details) providers are supported

## Credits ##
* icons: [Humano2 icon theme](http://schollidesign.deviantart.com/art/Human-O2-Iconset-105344123)
* Web framework: [Wt](http://www.webtoolkit.eu/wt)
* CSS framework: [Twitter Bootstrap](http://twitter.github.io/bootstrap)

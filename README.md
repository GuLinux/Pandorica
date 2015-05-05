# Pandorica #
Simple yet powerful Web Media Player

This HTML5-based Media Player will allow you to stream your multimedia collection in a local network, or over the web.
It supports the most popular web media formats, such as
 * mp3
 * ogg
 * ogv
 * webm
 * mp4 (audio/video)
 * flv

It also supports subtitles, file searching via media title or file name, media cover selection and much more.

It's written in C++ (C++11,  to be precise), using [Wt](http://www.webtoolkit.eu/wt), [Twitter Bootstrap](http://twitter.github.io/bootstrap), [MediaElement.js](http://mediaelementjs.com/) and [Video.js](http://www.videojs.com)

## Dependencies ##
 * [gcc](http://gcc.gnu.org/gcc-4.7/) 4.7 or above
 * [Wt](http://www.webtoolkit.eu/wt) 3.3.1 or above
 * [Boost](http://boost.org)
 * [GraphicsMagick++](http://www.graphicsmagick.org/Magick++/) ([ImageMagick++](http://www.imagemagick.org/script/index.php) should work too, but I found it much less reliable)
 * [Qt4/5](http://qt-project.org/) (Optional, to add a system tray icon when your server is running)
 * libpng and libjpeg (the last one is optional, but really reccomended) for thumbnails generation
 * zlib (or libz on some systems)
 * yasm or nasm (to build builtin ffmpeg libraries)


Also make sure to have a recent gcc version supporting C++11.

On a typical Ubuntu machine you need to do the following:

 * `sudo apt-get install build-essential cmake yasm` (compilers and tools)
 * `sudo apt-get install git` (required if you want to clone the git repository)
 * `sudo apt-get install libpng12-dev zlib1g-dev libjpeg8-dev libgraphicsmagick++1-dev libboost1.55-all-dev libwt-dev libwtdbo-dev`
 
at least one of

 * `sudo apt-get install libwtdbosqlite-dev`
 * `sudo apt-get install libwtdbopostgres-dev`
 
 Optionally, to have Qt integration (system tray icon plus other improvements), install one of these:
 * `sudo apt-get install libqt4-dev`
 * `sudo apt-get install qtbase5-dev`
 


### Database ###
At least one of
 * [Sqlite3](http://www.sqlite.org/)
 * [PostgreSQL](http://www.postgresql.org/)

Mysql/MariaDB will probably be supported soon.


## Quick Start ##
First, make sure to have all dependencies installed (if you are compiling from source code, you need the headers packages too).

Then, clone the repository and update the submodules.

    `git clone https://github.com/rockman81/Pandorica.git`
    `cd Pandorica/`
    `git submodule init`
    `git submodule update`

Configure and build *Pandorica*

    `mkdir build`
    `cd build`
    `cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr`
    `make`
    `sudo make install'

Then, just run the '*Pandorica*' command. When used with no arguments,  *Pandorica* will run on [localhost:8080](http://localhost:8080), and store your media database in $HOME/.config/Pandorica/Pandorica.sqlite

## Mobile ##
*Pandorica* is already mostly mobile ready.
You need to add the following code:

  `<meta-headers user-agent=".*">`
  `  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1" />`
  `</meta-headers>`


in your wt_config.xml file, outside the "properties" section.


## Advanced configuration and customization ##
Use the --help option for the most common options,  and the --help-full option to see all command line options (including Wt default ones).
Lookup the official [Wt Reference Page](http://www.webtoolkit.eu:3080/wt/doc/reference/html/overview.html#wthttpd) for a detailed overview.

*Pandorica* also uses some extra configuration properties.
You can set them in the "properties" section of your *wt_config.xml* file (usually located in /etc/wt, you can use a different one with the "-c" option).

### Running in an existing webserver ###
If you want to publish your *Pandorica* box over the internet it is highly reccomended to run it inside an existing web server installation (like apache, lighttpd, nginx)
Follow these steps:
 * Configure your webserver to forward requests to *Pandorica*. You can achieve this by configuring *mod_proxy* in your webserver.
 * Configure some alias paths in your webserver. For instance, you will need the Wt resources folder (/usr/share/Wt/resources/) and *Pandorica* static resources (/usr/share/Pandorica/static/). Wt resources should be aliased as "/resources" on your webserver (you can configure it in your wt_config.xml file). You can choose *Pandorica* static resources path, and tell *Pandorica* this path with the --static-deploy-path argument.
 * Optionally, but reccomended, configure a PostgreSQL connection (see below)
 * Also optional, select a different http port in *Pandorica* if the default (8080) is busy with the --http-port argument
After completing these steps, you can run *Pandorica* in managed mode:
    `Pandorica --server-mode managed --static-deploy-path /pandorica-static`
Or, if you configured your webserver mod_proxy to deploy *Pandorica* as a subdirectory:
    `Pandorica --server-mode managed --static-deploy-path /pandorica-static --deploy-path /Pandorica`


### PostgreSQL connection parameters ###
Example property entry:

    `<property name="psql-connection">application_name=streamingapp host=localhost port=5432 dbname=streaming_app_database user=streamingapp_pg_username password=streamingapp_pg_password</property>`

If *psql-connection* property is set, *Pandorica* will **not** use the Sqlite3 connector.

### Authentication and Email settings ###

Email verification (strongly suggested if you are deploying your *Pandorica* box on the internet):

    `<property name="email-verification-mandatory">true</property>`

Email addresses:

    `<!-- Streaming App settings -->
    `<property name="auth-mail-sender-name">Streaming App</property>`
    `<property name="auth-mail-sender-address">admin@localhost</property>`
    `<!-- put a REAL address here, you will have to receive warnings here! -->`
    `<property name="admin-mail-name">StreamingApp Admin</message>`
    `<property name="admin-mail-address">admin@localhost</message>`

Remember to install a local SMTP server to relay emails, particularly if you set email-verification-mandatory true.
If you don't,  your users will not be able to login!

You can also enable OAuth authentication: [Google](http://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1Auth_1_1GoogleService.html#details) and [Facebook](http://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1Auth_1_1FacebookService.html#details) providers are supported

### Other useful properties ###
 * quit-password: if you define this property, you can quit or restart the application by calling either:

        http://pandorica_url/graceful-quit?pwd=<your-password> (waits for existing sessions to shutdown)
        http://pandorica_url/force-quit?pwd=<your-password>
        http://pandorica_url/graceful-restart?pwd=<your-password> (waits for existing sessions to shutdown) 
        http://pandorica_url/force-restart?pwd=<your-password>

## Credits ##
 * Web framework: [Wt](http://www.webtoolkit.eu/wt)
 * CSS framework: [Twitter Bootstrap](http://twitter.github.io/bootstrap)
 * icons: [GeomIcons](https://www.iconfinder.com/iconsets/geomicons), [freecns-cumulus](https://www.iconfinder.com/iconsets/freecns-cumulus)

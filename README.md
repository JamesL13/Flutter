# Flutter
### 0.3.0

An HTTPS/SPDY server with the ultimate compromise between speed and
flexibility. Fast, direct, light, and powered by [LUA](http://www.lua.org/) and
[libmicrohttpd](http://www.gnu.org/software/libmicrohttpd/). Secure connection
and some postgres examples are next in the roadmap.

It comes under the MIT license, so happy hacking!

## Usage

### Get These

 - [cmake](http://www.cmake.org/) build system
 - GNU [libmicrohttpd](http://www.gnu.org/software/libmicrohttpd/)
 - PostgreSQL [libpq](http://www.postgresql.org/docs/9.4/static/libpq.html)

### Clone

    git clone https://github.com/grim-fandango/Flutter.git
    cd Flutter

### Build

    mkdir build
    cd build
    cmake ..
    make
    bin/Flutter ../tests/test.lua
    
Open a browser, go to `localhost:8080`.

## Why?
Simple: when we write web apps, what we want is _very, very simple_:

 - The ability to _change things very rapidly_ and redeploy.
 - The ability to do things easily in _very few lines of code_.
 - The ability to _scale transparently_.
 - The ability to _do things very cheaply_.

There are no other web frameworks or servers which can accomplish all of these things without many frustrating caveats. The achilles heel of basically every heavily developed server is that it is one of the following:

 - Heavy configuration
 - Heavy resource consumption
 - Difficult deployment
 - Slow

Obviously vulnerability is not on that list. Indeed, that has been the primary concern of every open source server project of merit. Flutter aims to be different by promising to leverage only already heavily supported and tested software projects (such as LUA, LuaSQL, libmicrohttpd, ...), thus operating safely at it's core, but with an emphasis on minimalism and rapid deployment.

Flutter will always be a LUA interface for HTTP, \*SQL, and JSON. LUA itself is extremely fast and light compared to other interpreted solutions, and only heavily optimized and field tested libraries and code will be used in releases. Coverity static analysis, regular builds, unit testing, and a fuzzer will be used to ensure that from the _0.6.x stable release_, Flutter is totally robust.

## Help out

Help write Flutter! If you find a bug, file an issue report and write a patch if you're feeling particularly adventurous. If you have an idea, bother me about it or fork the repository and implement it yourself! If I like it enough I will bring it in to core.

### Coding

Just a couple of rules on this project:

 - Fat is ugly.
 - Try things in agile sprints.
 - Cleanup is incremental.
 - If you're adding more than 500 lines for a single feature, ask yourself if that feature is worth 500 lines, and ask yourself if you'd really use this feature in more than one end user project.
 - Document what you're doing! I don't care if it's super clear, just help yourself reason and mention things that YOU don't think are so obvious. I frequently leave lots of comments about what's going on with the LUA stack.
 - Ask yourself if it could be written in LUA.
 - If it could be, but it would be slow, then consider _writing a LUA module and embedding it in the Flutter LUA environment_.

## Use it

Flutter needs to see some action! If you are considering building a small server, perhaps just to serve some static content, or run a fully fledged web-app, the experience you gain for Flutter and yourself will benefit all developers. Feedback is always welcome, and I accept mail at wgwhitacre1 _at_ cougars _dot_ ccis _dot_ edu .

## As of 0.3.0, we support

 - HTTP connections
 - Full exploit safe HTTP transmission to LUA table translation in both directions.
 - Secure LUA panic and resume.
 - Large POST payload handling.
 - Compliant with HTTP/1.1 to the extent of [libmicrohttpd](http://www.gnu.org/software/libmicrohttpd/)

## As of 0.4.0, we will support

 - HTTPS support
 - client certificate authentication
 - session cookie management
 - LUA coroutine threading architecture
 - Freeform LUA table ORM
 - Magic reload when script file changes

## To infinity and beyond...

 - SPDY support
 - Simple reverse proxy mode
 - Replace LUA allocs with (podalloc)[https://github.com/grim-fandango/podalloc] to the greatest extent possible
 - Advanced security and scaling controls
 - MOSIX clustering tools

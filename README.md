sfivt
=====

Simple Framebuffer Image Viewing Tool for Linux. Displays an image (surprise) on the linux **console framebuffer**.
So you need to use <CTRL>+<ALT>+<F1-6> to switch to a real console, not that window-thingy you're usually using..

License
========
[GPLv2](http://opensource.org/licenses/GPL-2.0), see [LICENSE.md](https://github.com/HorstBaerbel/sfivt/blob/master/LICENSE.md)

Building
========

**Use CMake:**
```
git clone http://github.com/HorstBaerbel/sfivt
cd sfivt/src
cmake .
make
```

GCC 4.7 is needed to compile. You also need the FreeImage library. Install it with:
```
sudo apt-get libfreeimage-dev
```

Usage
========

```
sfivt [OPTIONS] <FRAMEBUFFER_DEVICE> <IMAGE_FILE>
```  
The FRAMEBUFFER_DEVICE should be something like /dev/fb0. If you can not access your framebuffer devices try it as super-user or add your user name to the "video" group.  
IMAGE_FILE should be the full path to an image file on disk. The sfivt can display all the formats the FreeImage library is able to read, so PNG/JPG/TIFF/BMP/GIF/TGA should be working.  

**Valid command(s):**  
- -1 One-shot mode. Exit directly after displaying the image. Do not wait for <ENTER>. Obscure, I know.  

**Examples:**  
Display an image on fb2 and directly exit: ```sfivt -1 /dev/fb1 ~/xxx/aaa.jpg```  

I found a bug or have suggestion
========

The best way to report a bug or suggest something is to post an issue on GitHub. Try to make it simple. You can also contact me via email if you want to.

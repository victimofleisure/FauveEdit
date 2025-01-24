# FauveEdit

## Apply the Fauve pseudo-color effect to images

The Fauve effect pseudo-colors pixels according to how popular their original color values are in the image. The algorithm increases contrast, enhances edges, adds texture, alters the palette, and reduces spatial cohesion. Here are some [examples](http://ffrend.blogspot.com/2011/12/good-news-everyone-ffrend-project-has.html).

The project was originally a [Freeframe plugin](https://github.com/victimofleisure/ckffplugs) in 2011. In 2022 it became a desktop application and various enhancements followed, including:

* Cropping
* Luma histogram with adjustable min and max levels
* Hue shifting with animation
* Video export
* Parallel processing to improve render speed

in 2025 the application's view was ported from GDI to Direct2D 1.1 for improved scaling performance and quality. Consequently Windows 7 SP1 is the oldest supported version.

The code compiles cleanly in Visual Studio 2012 and 2019. Wrapper classes for ID2D1DeviceContext are included.

# Links

* [Chris Korda software](https://victimofleisure.github.io/software)


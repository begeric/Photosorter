# Photosorter
Easily select and sort your photographs

The goal is to have a software to easily choose, prune, select and sort a large collection of photograhs.

![demo](http://g.recordit.co/JKj4mTGMxQ.gifg)

### The interface
* One "input list" where we can import pictures from a file.

* A collections of "selection lists" where selected pictures from the input list or another selection list are transfered too. 
  It is possible to then sort, re-prune, move to other lists and to save to files the pictures in those lists.
  
* A "main" view where the selected pictures from any of the list is shown.

### Image selection
* Use the space key to 'select' an image and show the next one. Use the arrows to naviguate in the lists otherwise. 
 'select' means 'move to another list'

### Sorting
* When using the sorting mode, the user is prompted to chose the best image among a selection
 in the main view. To select the best one, simply click on it.
 this will re-iterate until the sorted order has been established.
 
* Clicking on an image on any of the lists will pause the sorting process. It is possible to resume it, as well as to stop it at anytime.

### Technicalities
* Thumbnails are cached, so as to save time upon loading.
  Known issue: if the underlying image changes then the thumbnail is not updated. it shouldn't be very common though.
  
* Images are cached and prefetched. This allows for more seamless naviguation between pictures.
  Known issues: Qt seems wacky in Image loading so the prefetched is not as ambitious as planned.
  
* We use merge sort for sorting even though it is not really the best for sorting pictures.
  (because A > B && B > C does not implies A > C, where '>' is the 'better image than' comparator).
 It would be better to use an n^2 algo but then it would be very time consuming.

### Compiling
* This was compiled using Qt creator (available for download for free at https://www.qt.io/download/).
* You will need to compile epeg (https://github.com/mattes/epeg) in order to compile Photosorted.

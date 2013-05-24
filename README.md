###README###

Sample kernel extension that demonstrates how to hide from kextstat and friends
by modifying the corresponding OSArray containing OSKext objects and the *\_kmod*
list.

Bugz/ToDo:

* find sLoadedKexts dynamically
* better solution for hardcoded offsets?!


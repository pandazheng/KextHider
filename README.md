###README###

Sample kernel extension that demonstrates how to hide from kextstat by modifying
OSArray containing OSKext objects for the running kexts.

Bugz/ToDo:

* additionally alter kmod list
* find sLoadedKexts dynamically
* better solution for hardcoded offsets?!


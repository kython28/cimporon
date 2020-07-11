
# Cimporon
This isn't special, this is only a code piece for my projects. This code piece is used to index personalized databases.

# How to use?
If you fancy using cimporon, you need copy the cimporon header to your project to use its functions. To use Cimporon you don't need high knowledge, because the Cimporon functions are very easy to use.

To get started, you need open and/or create a file:
```C
cimpo *file; // Cimpo object
file = openFile("Probe_file");
if (file == NULL){
  // Error
}
```
Well, now you can use all Cimporon functions. So, we'll some examples... If you want to add a key:
```C
uint64_t key = 30;
int64_t value = 666;
if (addValue(file, key, value)){
  // Error
}
```
Remember, you must set a value for your key and the key **must not be** 0. Ok, if you want to get the key value, you can do:
```C
uint64_t value = getValue(file, key);
```
To edit a value:
```C
if (editValue(file, key, new_value)){
  // Error
}
```
To remove a key:
```C
if (removeKey(file, key)){
  // Error
}
```
To finish, you must free the Cimporon object:
```C
closeCimpoFile(file);
```
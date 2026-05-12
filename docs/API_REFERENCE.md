# VYRONIX API Reference (v1.0)

VYRONIX provides a rich set of built-in functions via its Native Registry. Below is the documentation for all available functions.

---

## 🇧🇩 বাংলা নির্দেশিকা (Bengali Guide)

### ১. ইনপুট/আউটপুট (IO Operations)
- `print(val)`: স্ক্রিনে যেকোনো ভ্যালু প্রিন্ট করে।
- `input()`: ইউজার থেকে ইনপুট নেয়।
- `log(val)`: ডিবাগিং এর জন্য ভ্যালু লগ করে।

### ২. অ্যারে এবং স্ট্রিং (Arrays & Strings)
- `len(obj)`: অ্যারে বা স্ট্রিং-এর দৈর্ঘ্য রিটার্ন করে।
- `push(arr, val)`: অ্যারের শেষে নতুন ডাটা যোগ করে।
- `pop(arr)`: অ্যারের শেষ থেকে ডাটা রিমুভ করে।
- `sort(arr)`: অ্যারেকে সর্ট (Sort) করে।
- `toUpper(str)` / `toLower(str)`: স্ট্রিংকে বড় বা ছোট হাতের অক্ষরে রূপান্তর করে।

### ৩. গণিত (Math)
- `abs(x)`: পরম মান (Absolute value) বের করে।
- `sqrt(x)`: বর্গমূল বের করে।
- `random()`: ০ থেকে ১ এর মধ্যে একটি র‍্যান্ডম নম্বর দেয়।

---

## 🇬🇧 English Documentation

### 1. IO Operations
- `print(any)`: Prints the value to standard output.
- `input()`: Reads a line from stdin.
- `log(any)`: Prints a formatted log message.

### 2. Array & String Utilities
- `len(array|string)`: Returns the number of elements or characters.
- `push(array, any)`: Adds an element to the end of the array.
- `pop(array)`: Removes and returns the last element.
- `sort(array)`: Sorts the array in-place.
- `split(string, delimiter)`: Splits a string into an array.
- `trim(string)`: Removes whitespace from both ends.
- `contains(string, substring)`: Returns true if substring is found.

### 3. Math Functions
- `abs(num)`: Returns absolute value.
- `sqrt(num)`: Returns square root.
- `pow(base, exp)`: Returns base raised to exponent.
- `min(a, b)` / `max(a, b)`: Returns minimum or maximum of two values.
- `random()`: Returns a pseudo-random float between 0.0 and 1.0.

### 4. System & Advanced
- `assert(bool)`: Throws an error if condition is false.
- `loadLibrary(path)`: (Experimental) Loads a dynamic library (.dll/.so).
- `time()`: Returns current Unix timestamp.

---

## 🚀 Example Usage

```vyronix
// Array Example
var list = [10, 5, 8];
push(list, 20);
sort(list);
print(len(list)); // Output: 4

// String Example
var name = "  Vyronix  ";
print(toUpper(trim(name))); // Output: VYRONIX
```

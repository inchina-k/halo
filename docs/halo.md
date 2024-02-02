# Halo programming language

Halo - high-level general-purpose intepreted multiparadigm programming language  

---

## Comments

```
# This is a comment
```	
	
## Operators

### Unary

```
-  
```

### Binary

```
=, +, -, *, /, %  
>, <, ==, <=, >=, !=, +=, -=, *=, /=, %=  
```

### Logical

```
not  
and  
or  
```

## Types

```
int  
float  
str
bool
```  

## Assignment

```
let x = 0;  
```

## Initialization

```
var p;                   # null value by default  
var p = Point(1.0, 2.0); 
``` 

## Control Statements

### if

```
if x > 0 and x < 10:  
    print(x);  
end  
```

### if-else

```
if x > 0 and x < 10:  
    print(x);  
else:  
    print(x + 10);  
end  
```

### if-elif

```
if x and not y:  
    print(x);  
elif x < 0:  
    print(x - 10);  
else:  
    print(x + 10);  
end  
```

### while

```
while x < 10:  
    print(x);  
    x += 1;  
end  
```

### for

```
for x in range (1, 10, 2):  
    print(x);  
end  
```

### try-catch

```
try:  
    ...  
catch Exception1 e:  
    print(e.msg());  
catch Exception2 e:  
    print(e.msg());  
finally:
    print("hey");
end  
```

## Functions

```
fun factorial(n):  
    if x == 0 or x == 1:  
        return 1;  
    end  
	
    return n * factorial(n - 1);  
end  
```

## Classes

```
class Point:  
    fun _init_(x, y):  
        my.x = x;  
        my.y = y;  
    end  
	
    fun _init_(x, y, z):  
        my.x = x;  
        my.y = y;  
        my.z = z;  
    end  
	
    fun _str_():  
        return "(" + str(my.x) + ", " + str(my.y) + ")";  
    end  
	
    fun dist(other):  
        return pow(pow(my.x - other.x, 2) + pow(my.y - other.y, 2), 0.5);  
    end  
end  # Point  

pair_1 = Pair(1.0, 2.0);  
pair_2 = Pair(3.0, 4.0);  
```

## Lambda

```
var p = lambda(x, y): x * y; end  
print(p(2, 3));  
```

## List

```
let i_am_list = [1, 2, 3]
var another_list = list(42, "hello")
let x = i_am_list[2]
print((i_am_list))
```

## Cast

```
to_int("10");
to_str(10);
```
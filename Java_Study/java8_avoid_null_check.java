//http://winterbe.com/posts/2015/03/15/avoid-null-checks-in-java/

class Outer {
    Nested nested;
    Nested getNested() {
        return nested;
    }
}
class Nested {
    Inner inner;
    Inner getInner() {
        return inner;
    }
}
class Inner {
    String foo;
    String getFoo() {
        return foo;
    }
}
//Resolving a deep nested path in this structure can be kinda awkward.
//We have to write a bunch of null checks to make sure not to raise a NullPointerException:

Outer outer = new Outer();
if (outer != null && outer.nested != null && outer.nested.inner != null) {
    System.out.println(outer.nested.inner.foo);
}

//We can get rid of all those null checks by utilizing the Java 8 Optional type. 
//The method map accepts a lambda expression of type Function and automatically 
//wraps each function result into an Optional. 
//That enables us to pipe multiple map operations in a row.
//Null checks are automatically handled under the hood.

Optional.of(new Outer())
    .map(Outer::getNested)
    .map(Nested::getInner)
    .map(Inner::getFoo)
    .ifPresent(System.out::println);

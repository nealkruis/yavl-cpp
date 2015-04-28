# NEWS! Data Binding Alpha Released #

Alpha release of data binding for YAML in C++ is available!! Check out the DataBinding documentation or browse the [source](http://code.google.com/p/yavl-cpp/source/browse/branches/yatc/src/yatc.cpp) [code](http://code.google.com/p/yavl-cpp/source/browse/branches/yatc/src/yatc.h) (may need a few readability cleanups). Check out the branch 'yatc' to get it before I make a release.

Direct link to validator module [source](http://code.google.com/p/yavl-cpp/source/browse/trunk/src/yavl.cpp) [code](http://code.google.com/p/yavl-cpp/source/browse/trunk/src/yavl.h).

## Summary ##

YAVL-CPP is a C++ library to help validate YAML files
against a user-provided specification.

The specification file is also written in YAML.

Note: this is not a tool to check if the input file is a valid
YAML; any YAML parser will do that. This library can be
used to check if the input YAML has the tree structure
(i.e., keys, values, types, nesting of keys) that your
application expects. See example below.

Structures such as lists of lists, maps of lists,
lists etc can be nicely checked. :)

## Developer Notes ##

What I found interesting about this exercise was how short I was
able to make the checker program by a judicious selection of:

  * problem space restriction
  * treespec language is itself YAML
  * recursion

The clever idea here (if it can be called so) is that the specification
language has been chosen and the problem has been restricted so that the
tree representing the specification is structurally similar to the tree
representing the data to be checked. Look at the example specification
the valid file below.

This simplifies the checking algo: the base case is checking a leaf node
specification; other checks can be achieved by doing a lockstep in-order
traversal of both trees.

Since YAML is basically a tree, and trees lend them to recursive
solutions, I am able to check things like arbitrary nesting
of structures in relatively small source code size.

A similar recursive algorithm is used for the data binding code generator
also.

## Example Tree Specification ##

```
map:
  HEADER:
    map:
      name: [string: ]
      version: [string: ]
      size: [enum: [big, small]]
      pieces:
        map:
          a:
            list: [string: ]
          b:
            list: [uint64: ]
```

Notice that HEADER.pieces.a is list nested within
the map HEADER.pieces. You can have arbitrary levels
of nesting, and also do things like maps within lists.

Note: one of the items in the [TODO](http://code.google.com/p/yavl-cpp/source/browse/trunk/TODO) is to simplify this syntax.

## Valid YAML ##

```
HEADER:
  name: myname
  version: 1.02
  size: big
  pieces:
    a: [hello, world]
    b: [100, 200]
```

## Invalid YAML ##

```
HEADER:
  name: myname
  version: 1.02
  size: xbig
  pieces:
    a: [hello, world]
    b: [x100, 200]
```

When you run invalid YAML through the validator, it will detect two errors ('xbig' not allowed, and 'x100' isn't numeric). Your app has the option to print the library-generated error message. If you run the example checker program (see below), this is the output you can expect to see:

```
REASON: unable to convert 'x100' to 'unsigned long long'.
  doc path: HEADER.pieces.b[0]
  treespec path: map.HEADER.map.pieces.map.b.list

REASON: enum string 'xbig' is not allowed.
  doc path: HEADER.size
  treespec path: map.HEADER.map.size
```

## Dependencies ##

Yavl-cpp uses the excellent [yaml-cpp](http://code.google.com/p/yaml-cpp/) library. Go check out yaml-cpp just for the heck of it. It's one of the
best designed libraries out there (IMHO). Simple, and to the
point implementation and documentaion.

## Compilation ##

Nothing fancy. Somehow compile in yavl.cpp and all the cpp files of
yaml-cpp with your program. If your program is simple, just specify all those files
as part of your g++ command line. Or make a library archive and link that it.
Or separately compile the .cpp files into .o object files and specify them in
you g++ command line. I believe that libraries should pollute
your build process as little as possible (thankfully, so does
yaml-cpp).

Here is how you would compile the example checker program,
example-code/checker.cpp. YAML\_CPP\_DIR is where you
untarred the yaml-cpp source and YAVL\_CPP\_DIR is where
you untarred the yavl-cpp source.

```
g++ -I$YAML_CPP_DIR/include -I$YAVL_CPP_DIR/src \
  $YAML_CPP_DIR/src/*.cpp \
  $YAVL_CPP_DIR/src/yavl.cpp \
  $YAVL_CPP_DIR/example-code/checker.cpp -o checker
```

And run it like this:

```
checker $YAVL_CPP_DIR/example-specs/gr3.yaml $YAVL_CPP_DIR/example-specs/y0.gr3.yaml
```

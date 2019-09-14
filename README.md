# IdTool
A utility for viewing and manipulating SQRL identities.

![IdTool Main Window](assets/screenshot_1.png)

## Features

### Visual representation of SQRL identities
SQRL employs its own binary identity storage format called "S4". While being lightweight, compact and extensible, S4 binary data is not readily human-readable, unlike alternative formats such as Json and Xml. This makes it hard for users and developers alike to quickly check the contents of a SQRL identity stored in the S4 data format.

IdTool shows a nice visual representation of each of the identity's binary blocks and allows for a quick inspection of the values encoded within the identity.

### Manipulating identity files
IdTool lets the user 
 - change all values encoded within the S4 container
 - add and delete identity blocks
 - rearrange identity blocks
 - and finally save the modified data back to disk. 

This makes it an ideal tool for creating "rogue" identity files, which can be useful for hardening the parsing code of any SQRL client application.


### Parsing of custom identity blocks
IdTool does not employ simple static parsing of the S4 format. 

Instead, it uses **json templates** for dynamically parsing identity blocks. IdTool comes with pre-built json templates for all currently known block types (Types 1, 2 and 3).

This approach makes it possible to parse and display custom identity block types only by supplying the corresponding json template.

This is an example of how such a template looks like:
```json
{
  "block_type": 99,
  "description": "Add a nice description of your block type",
  "color": "rgb(252, 238, 199)",
  "items": [
    {
      "name": "Length",
      "description": "Inclusive length of the entire outer block",
      "type": "UINT_16",
      "bytes": 2
    },
    {
      "name": "Type",
      "description": "Block type",
      "type": "UINT_16",
      "bytes": 2
    },
    {
      "name": "Your own custom block item",
      "description": "Give a useful description",
      "type": "BYTE_ARRAY",
      "bytes": 32
    },
	...
  ]
}
```

If you are interested in creating your own block templates, I recommend to look at the standard templates provided with the app (within the _blockdev/_ subdirectory) to get a sense for how data types, repitition and dynamic item lengths are handled.


## Platforms
IdTool was written in platform neutral C++ using the [Qt Framework](https://www.qt.io) and it should therefore be possible to compile it for all platforms supported by the Qt framework. Among those are the Windows, Linux and MacOS.

IdTool is currently being developed and tested on Windows only, any help with testing it on other platforms is highly appreciated.

## Roadmap
There are some more ideas for features that I would still like to implement if I find time, like for example building json templates using the GUI, diffing of identities and creating site-specific public keys to help in debugging identity-related problems.

## Collaboration
I invite everyone willing to contribute to do so by either helping with further development/posting PRs, or simply finding and reporting bugs using the issue tracker.

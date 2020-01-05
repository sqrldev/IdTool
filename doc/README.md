# IdTool Documenation

This is the source code documentation for the _IdTool_ utility app.

You can browse the documentation by cloning or downloading the repository and then opening the file `doc/html/index.html` in a browser.

## (Re-)building the documenation
This project uses [Doxygen](http://www.doxygen.nl/) to generate its documentation directly from the source files. 

Before (re-)building the documentation, make sure that Doxygen is properly installed and the path to the doxygen executable was added to the system's (or user's) `PATH` environment variable.

Then, you can run the `create_docs.bat` batch file from the `doc/` subdirectory. (Windows only)

On Linux, just run the doxygen executable from the `doc/` directory, specifying `doxyfile` as the configuration file.

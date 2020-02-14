# Adding new tests

1) Create new directory with the name of test
2) Create source files inside newly created directory
3) Add new file called Makefile containing the list of additional sources to be compiled (external files), test-specific defines or inclusions. Do not add files inside the test directory to the source list - they will be added automatically. See any Makefile in this directory for example.
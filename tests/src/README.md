# Adding new tests

1) Create new directory named test_<test_name>
2) Create source files inside newly created directory
3) If the test depends on *external* sources, add new file called Sources.mk containing the list of additional sources to be compiled. Do not add files inside the test directory to the source list - they will be added automatically. See any Sources.mk file for example.
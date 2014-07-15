if (!FileFind("HOME/Budget",,FUF_JUST_DIRS)) {
  MkDir("HOME/Budget");
  Copy("/Apps/Budget/Accts.TXT.Z","HOME/Budget");
}

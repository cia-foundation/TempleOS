Cd(__DIR__);;
if (!FileFind("~/Budget",,FUF_JUST_DIRS)) {
  MkDir("~/Budget");
  Copy("Accts.DD.Z","~/Budget");
}

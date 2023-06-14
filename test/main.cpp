
import boost.ut;

__attribute__((constructor((0)))) inline void foo(int argc, const char *argv[]) {
  ::boost::ut::detail::cfg::largc = argc;
  ::boost::ut::detail::cfg::largv = argv;
}

int main(/*int , const char *[]*/) {
  // boost::ut::detail::cfg::parse(argc, argv);
}

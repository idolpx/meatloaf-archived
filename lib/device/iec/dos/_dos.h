// //
// // https://en.wikipedia.org/wiki/Commodore_DOS
// // https://www.devili.iki.fi/pub/Commodore/docs/books/Inside_Commodore_DOS_OCR.pdf
// // http://www.n2dvm.com/UIEC.pdf
// //


// #include <cstdint>
// #include <string>
// #include <ctime>

// struct dos_file
// {
//     std::string media;
//     std::string path;
//     std::string name;
//     std::string type;
// };

// struct dos_command
// {
//     dos_file file_target;
//     dos_file file_source;
//     std::string mode;
//     std::string command;
//     bool replace = false; // @ - Save Replace
//     uint16_t datacrc = NULL;
// };

// class DOS
// {
// public:
//     std::string version;
//     dos_command command;
//     time_t date_match_start = NULL;
//     time_t date_match_end = NULL;

//     virtual void execute(std::string command) = 0;
// };
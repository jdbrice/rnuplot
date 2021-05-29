#include "cxxopts.h"

#include <fstream>
#include <iomanip>
using namespace std;

#include <array>
#include <sys/stat.h>

bool use_cache = true;

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

void process_file( std::string file );
void process_root2gnuplot_line( std::string line );

int main(int argc, char** argv)
{
	cxxopts::Options options("rnuplot", "Directly plot ROOT data with gnuplot");

	options.parse_positional({"file"});
	options.show_positional_help();
	options.positional_help( "[FILE]" );


	options.add_options()
		("c,cache", "Use cached data, set to false to always convert", cxxopts::value<bool>()->default_value("true"))
		("f,file", "File", cxxopts::value<std::string>())
		("d,debug", "Enable debugging", cxxopts::value<bool>()->default_value("false"))
		// ("f,foo", "Param foo", cxxopts::value<int>()->default_value("10"))
		("h,help", "Print usage")
	;


	auto result = options.parse(argc, argv);
	


	if (result.count("help"))
	{
	  cout << options.help() << endl;
	  exit(0);
	}
	bool debug = result["debug"].as<bool>();
	string bar;
	if (result.count("bar"))
	  bar = result["bar"].as<string>();
	// int foo = result["foo"].as<int>();

	string file;
	if (result.count("file"))
	  file = result["file"].as<string>();
	cout << "file: " << file << endl;

	if ( result["cache"].as<bool>() == false ){
		cout << "cache = false" << endl;
		use_cache = false;
	}

	process_file( file );
	// return 0 ;
	string gnuplot_cmd = "gnuplot " + file;
	cout << gnuplot_cmd << endl; 
	exec( gnuplot_cmd.c_str() );

	return 0;
}


void process_file( std::string file ) {
	fstream infile;
	infile.open(file.c_str(),ios::in); 
	if (infile.is_open()){   //checking whether the file is open
		string line;
		while(getline(infile, line)){ //read data from file object and put it into string.

			std::size_t found = line.find("root2gnuplot");
			if (found!=std::string::npos)
				process_root2gnuplot_line( line );
		} // read file line-by-line
	infile.close(); //close the file object.
	}
} // process_file


void process_root2gnuplot_line( string line ){
	std::size_t found = line.find("root2gnuplot");
	// strip leading comment char and spaces
	std::string cmd = line.substr(found);
	

	stringstream opts;
	string exe, input, output, format;
	opts << cmd;
	opts >> exe >> input >> output >> quoted(format);

	bool skip = false;

	if ( use_cache ) {
		cout << "Checking for cached data in " << output << endl;
		struct stat buffer;   
		if (stat (output.c_str(), &buffer) == 0) {
			cout << "File " << output << " already exists, skipping converison. Use rnuplot --cache=false [FILE] to force conversion" << endl;
			skip = true;
		}
	}

	if ( !skip ){
		cout << cmd << endl;
		exec( cmd.c_str() );
	}
	
}
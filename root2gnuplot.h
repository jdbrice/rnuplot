//usr/bin/env root -l -b -q "$0( \"$1\", \"$2\", \"$3\" )"; exit $?
#include "TFile.h"
#include "TKey.h"
#include "TString.h"

#include "TH3.h"
#include "TH2.h"
#include "TH1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TTree.h"
#include "TCanvas.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdio>
#include <iomanip> // c++11
#include <cstdlib>
using namespace std;


const int col_width = 15;
const int col_precision = 5;
const std::string format_default_TH1 = "x y xlow xhigh ylow yhigh";
const std::string format_default_TH2 = "x y z";
const std::string format_default_TGraphErrors = "x y xlow xhigh ylow yhigh";

int verbosity = 1;

class DataFileMaker {
    public:

        struct PackedValues{
            double x, y, z;
            double xl, yl, zl;
            double xh, yh, zh;

            double dx(){
                return xh - x;
            }
            double dy(){
                return yh - y;
            }
            double dz(){
                return zh - z;
            }

        };

        DataFileMaker() {}
        ~DataFileMaker() {}

        std::vector<std::string> tmpfile_list; 
        size_t tmpfile_count_max = 0;
        size_t tmpfile_count = 0;

        void print_format( std::ofstream &tmp, string format ) {
            TString tsfmt = TString(format.c_str());
            TString token;
            Ssiz_t from = 0;
            tmp << "#";
            while ( tsfmt.Tokenize( token, from, " " ) ){
                tmp << std::left << std::setw( col_width ) ;
                tmp << token;
            }
            tmp << std::endl;
        }


        void format_line( std::ofstream &tmp, PackedValues &pv, string &format ){
            // NOW parse the format string and push data out in that order
            stringstream sstr( format );
            string token = "";
            string sep = "";
            while( std::getline(sstr, token, ' ') ){

                tmp << " " << sep << std::left << std::setw( col_width - 2 );
                if ( "x" == token )
                    tmp << pv.x;
                if ( "xlow" == token )
                    tmp << pv.xl;
                if ( "xhigh" == token )
                    tmp << pv.xh;
                if ( "y" == token )
                    tmp << pv.y;
                if ( "ylow" == token )
                    tmp << pv.yl;
                if ( "yhigh" == token )
                    tmp << pv.yh;
                if ( "z" == token )
                    tmp << pv.z;
                if ( "zlow" == token )
                    tmp << pv.zl;
                if ( "zhigh" == token )
                    tmp << pv.zh;
                if ( "dx" == token )
                    tmp << pv.dx();
                if ( "dy" == token )
                    tmp << pv.dy();
                if ( "dz" == token )
                    tmp << pv.dz();

                sep = " ";
            }
        }

        // master convert for all histogram obects (TH1, TH2, TH3)
        string convert( TH1 * h, string format, string fn, string meta = "") {

            std::ofstream tmp;
            
            tmp.open (fn, std::ofstream::out );
            


            TH3 * h3 = dynamic_cast<TH3*>( h );
            TH2 * h2 = dynamic_cast<TH2*>( h );


            if ( nullptr != h3 && h3->GetZaxis()->GetNbins() > 1 ){
                tmp << "#converted from TH3 " << meta << endl;
                convert_TH3( tmp, h3, format );
            } else if ( nullptr != h2 && h2->GetYaxis()->GetNbins() > 1 ){
                tmp << "#converted from TH2 " << meta << endl;
                convert_TH2( tmp, h2, format );
            } else {
                tmp << "#converted from TH1 " << meta << endl;
                convert_TH1( tmp, h, format );
            }

            tmp.flush();
            tmp.close();
            return fn; // return the filename
        }

        // master convert for all TGraph obects
        string convert( TGraph * g, string format, string fn, string meta = "" ) {
            std::ofstream tmp;
            tmp.open (fn, std::ofstream::out );
            
            tmp << "#converted from TGraph " << meta << endl;
            convert_TGraphErrors( tmp, g, format );

            tmp.flush();
            tmp.close();
            return fn;
        }


        void convert_TH1( std::ofstream &tmp, TH1*h, string format ){
            if ( verbosity > 0 ){
                cout << "Converting TH1" << endl;
            }
            if ( "" == format ){
                format = format_default_TH1;//"x y xlow xhigh ylow yhigh";
                cout << TString::Format("No format provided, default is: %s", format.c_str() ) << endl;
            }

            // Print the format to the file as column headers
            print_format( tmp, format );

            PackedValues pv;
            for ( int i = 1; i < h->GetNbinsX() + 1; i++ ){
                pv.x = h->GetBinCenter( i );
                pv.xl = h->GetBinLowEdge(i);
                pv.xh = h->GetBinLowEdge(i) + h->GetBinWidth(i);
                pv.y = h->GetBinContent( i );
                pv.yl = pv.y - h->GetBinError( i );
                pv.yh = pv.y + h->GetBinError( i );

                format_line( tmp, pv, format );

                tmp << std::endl;
            }

        } // convert_TH1

        void convert_TH2( std::ofstream &tmp, TH2*h, string format ){
            if ( verbosity > 0 ){
                cout << "Converting TH2" << endl;
            }

            if ( "" == format ){
                format = format_default_TH2;
                cout << TString::Format("No format provided, default is: %s", format.c_str() ) << endl;
            }

            // Print the format to the file as column headers
            print_format( tmp, format ) ;

            PackedValues pv;
            for ( int ix = 1; ix < h->GetNbinsX(); ix++ ){
                for ( int iy = 1; iy < h->GetNbinsY(); iy++ ){

                    // pack the TH2 data into struct and then print formatted
                    pv.x = h->GetXaxis()->GetBinCenter( ix );
                    pv.y = h->GetYaxis()->GetBinCenter( iy );
                    pv.z = h->GetBinContent( ix, iy );
                    pv.zh = pv.z + h->GetBinError( ix, iy );
                    pv.zl = pv.z - h->GetBinError( ix, iy );
                    pv.xl = h->GetXaxis()->GetBinLowEdge(ix);
                    pv.xh = pv.xl + h->GetXaxis()->GetBinWidth(ix);
                    pv.yl = h->GetYaxis()->GetBinLowEdge(iy);
                    pv.yh = pv.yl + h->GetYaxis()->GetBinWidth(iy);

                    format_line( tmp, pv, format );
                    tmp << std::endl;
                } // loop on iy
                // double blank line per gnuplot data format
                tmp << std::endl;
            } // loop on ix
        } // convert_TH2

        void convert_TH3( std::ofstream &tmp, TH3*h, string format ){
            //TODO
            cout << "TH3 is not implemented yet!" << endl;
        }


        void convert_TGraphErrors( std::ofstream &tmp, TGraph*gr, string format ){
            if ( verbosity > 0 ){
                cout << "Converting a TGraphErrors" << endl;
            }

            if ( "" == format ){
                format = format_default_TGraphErrors;
                cout << TString::Format("No format provided, default is: %s", format.c_str() ) << endl;
            }

            // Print the format to the file as column headers
            print_format( tmp, format );

            PackedValues pv;
            for ( int i = 0; i < gr->GetN(); i++ ){
                pv.x = gr->GetX()[i];
                pv.xl = pv.x - gr->GetErrorXlow( i );
                pv.xh = pv.x + gr->GetErrorXhigh( i );
                pv.y = gr->GetY()[i];
                pv.yl = pv.y - gr->GetErrorYlow( i );
                pv.yh = pv.y + gr->GetErrorYhigh( i );

                // print the formatted line
                format_line( tmp, pv, format );
            }

        } // convert_TGraph

};


const char tree_cmd_open = '{';
const char tree_cmd_close = '}';

TString next_draw_opt( TString &in ){
    int start = in.First( tree_cmd_open );
    int stop = in.First( tree_cmd_close );

    if ( stop < 0 )
        stop = in.Length();

    if ( start >= 0 ){
        // remove from input
        TString opt = in(start+1, stop - (start+1) );
        in = in(stop + 1, in.Length());
        return opt;
    }
    return "";
}



TString parse_tree_name( TString in ){
    if ( in.First(tree_cmd_open) > 0 ){
        return in(0, in.First(tree_cmd_open) );
    }
    return "";
}

TString parse_tree_obj_name( TString dcmd ){
    int start = dcmd.First(">>");
    int stop1 = dcmd.First("(");
    int stop2 = dcmd.First(tree_cmd_close);

    int stop = stop1;
    if ( stop1 < 0 )
        stop = stop2;
    if ( stop < 0 || start < 0){
        // not found, so add it and return
        dcmd = dcmd + " >>h";
        return "h";
    }

    // since ">>".Length() = 2
    TString name =  dcmd(start+2, stop - (start+2));
    return name.ReplaceAll( " ", "" );
}

void parse_input_string( TString input, TFile *&tfile, TObject *&obj ){

    int index_split = input.First( ':' );
    TString rfn = input(0, index_split);

    if ( verbosity > 0 ) {
        cout << "File: " << rfn.Data() << endl;
    }
    tfile = new TFile( rfn.Data() );

    TString hfn = input(index_split+1, input.Length());

    // TString dcmd = "";
    // parse tree options
    // input.root:ttree_name[y:x>>hwhatever(100, 0, 100)][z<4][option][max][first]

    // first check for any draw opts
    TString draw_options = hfn;
    TString dcmd   = next_draw_opt( draw_options );
    TString dquery = next_draw_opt( draw_options );
    TString dopt   = next_draw_opt( draw_options );
    TString dmax   = next_draw_opt( draw_options );
    TString dfirst = next_draw_opt( draw_options );
    TString tree_name = parse_tree_name( hfn );
    TString obj_name = parse_tree_obj_name( dcmd );
    
    obj = tfile->Get( tree_name.Data() );
    TTree * ttree = dynamic_cast<TTree*>(obj);
    if ( ttree != nullptr ){
        // cout << "got tree, drawing to \"" << obj_name.Data() << "\"" << endl;
        TCanvas * c = new TCanvas(); // this suppresses output about default canvas from ROOT
        c->Update(); // this sup compiler warning about c unused
        ttree->Draw( dcmd.Data(), dquery.Data(), dopt.Data() );
        cout << TString::Format( "TTree->Draw( \"%s\", \"%s\", \"%s\", %s, %s )", dcmd.Data(), dquery.Data(), dopt.Data(), dmax.Data(), dfirst.Data() ).Data() << endl;
        obj = (TH1*)gDirectory->Get(obj_name.Data());
    } else {
        // just get single object based on name
        obj = tfile->Get( hfn.Data() );
    }
} // parse_input_string

void root2gnuplot( TString rootfile_hist = "", TString output = "out.dat", TString format = "", int _verbosity = 10 ){
    verbosity = _verbosity;
    if ( verbosity >= 0 ){
        cout << "Usage:" << endl;
        cout << "\troot2gnuplot.C <--help> input.root<:name> output.dat <format>, if <:name> omitted, convert all" << endl;
        cout << "\t--help print more information" << endl;

        if ( rootfile_hist == "--help" ){
            cout << "help" << endl << endl;
            cout << "format specifiers (case sensitive):" << endl;
            cout << "\tx, y, z: coordinate from first, second, third axis for TH1, TH2, TH3 histograms" << endl;
            cout << "\t<axis>low: value on <axis> at lower error bar, example: xlow, ylow, zlow" << endl;
            cout << "\t<axis>high: value on <axis> at upper error bar, example: xhigh, yhigh, zhigh" << endl;
            cout << "\td<axis>: value on <axis> at upper error bar minus value on <axis>, example: dx, dy, dz" << endl << endl << endl;

            cout << "default formats:" << endl;
            cout << "\tTH1: " << format_default_TH1 << endl;
            cout << "\tTH2: " << format_default_TH2 << endl;
            cout << "\tTGraphErrors: " << format_default_TGraphErrors << endl << endl;

            cout << "TTrees:" << endl;
            cout << "\t<name> should have the following form:" << endl;
            cout << "\tttree_name{draw_command}{selection}{draw_options}" << endl;
            cout << "\tNOTE: if you have spaces in your command string, you must wrap the entire argument in (single) quotes" << endl << endl; 


            cout << "examples:" << endl;
            cout << "\tConvert a single object and output a single file:" << endl;
            cout << "\t``root2gnuplot.C input.root:h1 TEST.dat``" << endl;
            cout << "\t- This converts histogram named h1 in ROOT file input.root and outputs into a file named TEST.dat" << endl;
            cout << "\t" << endl;
            cout << "\tConvert all supported objects in a file and output one data file for each:" << endl;
            cout << "\t``root2gnuplot.C input.root data/SL``" << endl;
            cout << "\t- This will export all supported objects (using default formats) in the file using the output 'data/SL' as a prefix. " << endl;
            cout << "\t- Full output filename would be 'data/SL_<name>.dat' where '<name>'' is the ROOT object\'s name" << endl;
            cout << "\t- TODO: support overriding formats per type" << endl;
            cout << "\t" << endl;
            cout << "\tConvert directly from a TTree:" << endl;
            cout << "\t ``root2gnuplot 'input.root:tree_name{pt>>h1(100, 0, 10)}{eta < 5}' TEST.dat" << endl;
            cout << "\t- This projects the `pt` value of the TTree, applies the selection `eta < 5` and outputs into a new histogram `h1` with the bins specified" << endl;
            cout << "\t- The three sets of `{}` correspond to the options of the TTree::Draw(...) command. " << endl;
            return;
        }
    }

    
    

    bool convert_all = rootfile_hist.First( ':' ) == -1;
    int index_split = rootfile_hist.First( ':' );
    if ( index_split < 0 )
        index_split = rootfile_hist.Length();

    TString rfn = rootfile_hist(0, index_split);
    TString hfn = rootfile_hist(index_split+1, rootfile_hist.Length());

    if ( verbosity > 0 ) {
        cout << "ROOT Input File: " << rfn << endl;
        if ( hfn != "" )
            cout << "ROOT Input Object: " << hfn << endl;
        else 
            cout << "Converting all objects in file" << endl;
        cout << "Ouput File: " << output << endl;
    }

    TFile * rf = nullptr;
    TObject * obj = nullptr;
    parse_input_string( rootfile_hist, rf, obj );

    // TFile *rf = new TFile( rfn.Data() );
    if ( !rf || rf->IsOpen() == false ){
        cerr << "Cannot open input file: " << rf << endl;
        return;
    }

    // TObject * obj = rf->Get( hfn.Data() );
    if ( !obj &&  hfn != "" ) {
        cerr << "Cannot get object named: " << hfn << endl;
        return;
    }

    DataFileMaker dfm;
    string metadata = "";
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    stringstream dtss;
    dtss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");

    if ( obj ){
        metadata = "name: " + string(hfn.Data()) + ", file: " + string(rfn.Data()) + ", date: " + dtss.str();
        if ( dynamic_cast<TH1*>( obj ) ){
            if ( verbosity > 0 ){
                cout << "Converting '" << hfn << "' as a histogram" << endl; 
            }
            dfm.convert( dynamic_cast<TH1*>( obj ), format.Data(), output.Data(), metadata );
        } else if ( dynamic_cast<TGraph*>( obj ) ){
            if ( verbosity > 0 ){
                cout << "Converting '" << hfn << "' as a graph" << endl; 
            }
            dfm.convert( dynamic_cast<TGraph*>( obj ), format.Data(), output.Data(), metadata );
        }
        return;
    }

    // convert all objects in file
    if ( !obj && convert_all ) {
        TIter next(rf->GetListOfKeys());
        TKey *key;
        while ( (key = (TKey*)next()) ) {
            TString loutput = TString::Format( "%s_%s.dat", output.Data(), key->GetName() );
            if ( verbosity > 0 ){
                cout << "Converting " << key->GetName() << "[" << key->GetClassName() << "] into " << loutput << endl;
            }
            metadata = "name: " + string(key->GetName()) + ", file: " + string(rfn.Data()) + ", date: " + dtss.str();
            obj = rf->Get( key->GetName() );
            TGraph * g = dynamic_cast<TGraph*>( obj );
            TH1 * h = dynamic_cast<TH1*>( obj );

            if ( g != nullptr )
                dfm.convert( g, format.Data(), loutput.Data(), metadata );
            else if ( h != nullptr ){
                dfm.convert( h, format.Data(), loutput.Data(), metadata );
            }
        }
    }
}

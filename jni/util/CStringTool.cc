/**
 * @file CStringTool.cc
 * @author  lishujie@landforce
 * @version 1.0
 * @brief Cubic Util, Common Convert Util
 * @detail Cubic Util, Common Convert Util
 */

#ifndef _CCOMMONCONVERTUTIL_CC_
#define _CCOMMONCONVERTUTIL_CC_ 1

#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <cctype>
#include <vector>
#include <iomanip>

using namespace std;

class CStringTool
{
public:
    // ****************************** stl string *********************************
    // to upper or lower
    inline static string toLower( string str ) {
        for( string::iterator i = str.begin(); i != str.end(); i++ ) {
            *i = tolower( *i );
        }

        return str;
    };

    inline static string toUpper( string str ) {
        for( string::iterator i = str.begin(); i != str.end(); i++ ) {
            *i = toupper( *i );
        }

        return str;
    };

    // trim
    inline static string trimLeft( string str ) {
        string::iterator i;

        for ( i = str.begin(); i != str.end(); i++ ) {
            if ( !isspace( *i ) ) {
                break;
            }
        }

        if( i == str.end() ) {
            str.clear();
        }
        else {
            str.erase( str.begin(), i );
        }

        return str;
    };

    inline static string trimRight( string str ) {
        string::iterator i;

        for ( i = str.end() - 1; ; i-- ) {
            if ( !isspace( *i ) ) {
                str.erase( i + 1, str.end() );
                break;
            }

            if ( i == str.begin() ) {
                str.clear();
                break;
            }
        }

        return str;
    };

    inline static string trim( const string str ) {
        return trimRight( trimLeft( str ) );
    };

    // compare ignore case
    inline static int commpareIgnoreCase( const string &str1, const string &str2 ) {
        return toLower( str1 ).compare( toLower( str2 ) );
    };

    // start with
    inline static bool startWith( const string &str, const string &substr ) {
        return str.find( substr ) == 0;
    };

    // end with
    inline static bool endWith( const string &str, const string &substr ) {
        size_t i = str.rfind( substr );
        return ( i != string::npos ) && ( i == ( str.length() - substr.length() ) );
    };

    // replace
    static string replace( string str, const string &old_value, const string &new_value ) {
        while( true ) {
            string::size_type pos( 0 );

            if( ( pos = str.find( old_value ) ) != string::npos )
            { str.replace( pos, old_value.length(), new_value ); }
            else { break; }
        }

        return str;
    };

    static string replaceDist( string str, const string &old_value, const string &new_value ) {
        for( string::size_type pos( 0 ); pos != string::npos; pos += new_value.length() ) {
            if( ( pos = str.find( old_value, pos ) ) != string::npos )
            { str.replace( pos, old_value.length(), new_value ); }
            else { break; }
        }

        return str;
    };


    // to string
    template <class T>
    inline static string toString( const T &value ) {
        ostringstream oss;
        oss << setprecision( 8 ) << setiosflags( ios::fixed ) << value;
        return oss.str();
    };

    template<class T>
    inline static string toHexString( const T &value, int width ) {
        ostringstream oss;
        oss << hex;

        if ( width > 0 ) {
            oss << setw( width ) << setfill( '0' );
        }

        oss << value;
        return oss.str();
    }

    // from string
    template <class T>
    inline static T fromString( const string &str ) {
        T value;
        istringstream iss( str );
        iss >> value;
        return value;
    };

    template<class T>
    inline static T fromHexString( const std::string &str ) {
        T value;
        istringstream iss( str );
        iss >> hex >> value;
        return value;
    }

    // split
    static vector<string> split( const string &str, string sep = "," ) {
        vector<string> ret;

        if ( str.empty() ) {
            return ret;
        }

        string tmp;
        string::size_type pos_begin = str.find_first_not_of( sep );
        string::size_type comma_pos = 0;

        while( pos_begin != string::npos ) {
            comma_pos = str.find( sep, pos_begin );

            if( comma_pos != string::npos ) {
                tmp = str.substr( pos_begin, comma_pos - pos_begin );
                pos_begin = comma_pos + sep.length();
            }
            else {
                tmp = str.substr( pos_begin );
                pos_begin = comma_pos;
            }

            if ( !tmp.empty() ) {
                ret.push_back( tmp );
            }
        }

        return ret;
    }


    static string format( const char* fmt, ... ) {
        va_list args;
        va_start( args, fmt );
        int len = vsnprintf( NULL, 0, fmt, args );

        if( len <= 0 ) {
            return "";
        }

        char* buf = ( char* )malloc( len + 4 );
        memset( buf, 0, len + 4 );
        vsnprintf( buf, len + 1, fmt, args );
        va_end( args );
        string ret( buf );
        free( buf );
        return ret;
    }
	

};

#endif //_CCOMMONCONVERTUTIL_CC_
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

int main (int argc, char *argv[])
{
    std::string filename = "/Users/u5305887/Movies/cam2/analyse/12.csv";
    std::ifstream file (filename);
    std::string line;
    std::vector<int> beeid;
    std::vector<int> tag;
    std::vector<int> frame;
    std::vector<float> X;
    std::vector<float> Y;
    int i = 0;
    while ( file.good() )
    {
        
        if (i < 2) // remember capitalization
        {
            std::getline ( file, line, '\n' );
            //std::cout << value << '\n';
            i++;
            continue;
        }
        std::getline ( file, line, '\n' );
        //std::cout << i << " " << line << '\n';
        std::istringstream ss(line);
        std::string value;
        std::vector<std::string> line_values;
        while(std::getline(ss, value, ',')) {
            line_values.push_back(value);
        }
        beeid.push_back (std::stoi(line_values[0]));
        tag.push_back (std::stoi(line_values[1]));
        frame.push_back (std::stoi(line_values[2]));
        X.push_back (std::stof(line_values[3]));
        Y.push_back (std::stof(line_values[4]));
        /*

        if (id)
        {
            std::getline ( file, value, ',' );
            beeid.push_back (std::stoi(value));
            id = false;
            t = true;
        }
        else if (t)
        {
            std::getline ( file, value, ',' );
            tag.push_back (std::stoi(value));
            t = false;
            x = true;
        }
        else if (f)
        {
            std::getline ( file, value, ',' );
            frame.push_back (std::stoi(value));
            f = false;
            x = true;
        }
        else if (x)
        {
            std::getline ( file, value, ',' );
            X.push_back (std::stof(value));
            x = false;
            y = true;
        }
        else
        {
            std::getline ( file, value, ',' );
            Y.push_back (std::stof(value));
            y = false;
            id = true;
        }
        
        */
        //std::cout << value << '\n';
        //std::cout << i << " this" << '\n';

        /*
        beeid.push_back (std::stoi(value));
        std::getline ( file, value, ',' );
        tag.push_back (std::stoi(value));
        std::getline ( file, value, ',' );
        frame.push_back (std::stoi(value));
        std::getline ( file, value, ',' );
        X.push_back (std::stof(value));
        std::getline ( file, value, ',' );
        Y.push_back (std::stof(value));*/
        
        

        /*std::stringstream iss(line);

        for (int i = 0; i < 5; i++)
        {
            if (i < 5)
            {
                std::getline ( file, value, ',' );
                std::cout << value << '\n';
            }
            else
            {
                std::getline ( file, value, '\n' );
                std::cout << value << '\n';
            }
        }
        
        while(std::getline(iss, value, ',')) 
        {
            std::cout << value << '\n';
        }
                if (i > 20)
        {
            break;
        }
        */
        //std::cout << std::string( value, value.length()-1, value.length() ) << std::endl; // display value removing the first and the last character from it
        //std::cout << std::string(line) << std::endl;
        //break;

        i++;
    }

    std::cout << frame[15] << '\n';
    
    return 0;
}
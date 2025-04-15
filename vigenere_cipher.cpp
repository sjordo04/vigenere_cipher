#include <cstdlib>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
using std::string;
using std::vector;



string build_substring(int index, int m, string cipher){
    string substring;
    //create substring y_index
    for (int i = index; i < cipher.size(); i+=m){
        substring += cipher[(i)]; //append the ith char to the substring
    }
    return substring;
}

//cpp standard is modular arithmetic returns the negative (-6 % 26 = -6)
//this fixes that
int mod_positive(int a, int b) {
    int result = a % b;
    return result >= 0 ? result : result + b;
}

double index_of_coincidence(string text){
    std::vector<int> frequency(26, 0);
    int totalChars = 0;

    // Count frequency of each letter
    for (char c : text) {
        if (std::isalpha(c)) {
            frequency[std::toupper(c) - 'A']++;
            totalChars++;
        }
    }

    //calculate index of coincidence
    double sum = 0.0;
    for (int count : frequency) {
        // For each letter, calculate count * (count - 1)
        sum += count * (count - 1);
    }
    
    // Final formula: sum / (totalChars * (totalChars - 1))
    return sum / (totalChars * (totalChars - 1));
}

//returns the ioc for a length-guess of m
vector<double> calculate_ioc(int m, string cipher){
    vector<double> ioc;
    for (int i=0; i<m; i++){ //fill the array with ioc of substring_i, i = {1,...,m}
        ioc.push_back(index_of_coincidence(build_substring(i, m, cipher)));
    }
    return ioc;
}

//returns an array of substrings for a length-guess m
vector<string> build_substring_arr(int m, string cipher){
    vector<string> substrings;
    for (int i=0; i<m; i++){ //fill the array with ioc of substring_i, i = {1,...,m}
        substrings.push_back(build_substring(i, m, cipher));
    }
    return substrings; 
}

//crack the key, return a 2D array representing the values of Mg
vector<vector<double>> crack_key(int m, vector<string> substrings){
    std::vector<std::vector<double>> q(26, std::vector<double>(m, 0));
    for (int i = 0; i < m; i++){ //for each string y_i, 1 <= i <= m
        string text = substrings[i];
        for (char c : text) { //count the frequencies
            if (std::isalpha(std::toupper(c))) {
                q[(std::toupper(c) - 'A')][i]++; 
            }
        }
        for (int j = 0; j < 26; j++){ //q = f_j/N
            q[j][i] = (q[j][i]/substrings[i].length());
        }
    }
    //fill another vector with alphabet frequencies
    std::vector<double> p = {0.082, 0.015, 0.028, 0.043, 0.127, 0.022, 0.02, 0.061, 0.07, 0.002, 0.008, 0.04, 0.024, 0.067, 0.075, 0.019, 0.001, 0.06, 0.063, 0.091, 0.028, 0.01, 0.023, 0.001, 0.02, 0.001};

    std::vector<std::vector<double>> v_g(26, std::vector<double>(m, 0)); //2D vector of doubles
    for (int g = 0; g < 26; g++){ //for each 0 <= g <= 25
        for (int h = 0; h < m; h++){ //for each substring position
            for (int i = 0; i < 26; i++){ //for each letter frequency
                v_g[g][h] += p[i] * q[mod_positive(i + g, 26)][h]; //accumulate the dot product
            } 
        }
    }

    return v_g;
}

string find_key(vector<vector<double>> m_g, int m){
    string key;
    for (int i = 0; i < m; i++){
        double max = 0.0;
        int max_index = 0;
        for (int j = 0; j < 26; j++){
            if (m_g[j][i] > max){
                max = m_g[j][i];
                max_index = j;
            }
        }
        // Add the key letter to the key, converted back to ASCII
        key += (max_index + 'A');
    }
    return key;
}

//decrypt ciphered text
string decrypt(string cipher, string key){
    string plaintext;
    for (int i = 0; i < cipher.size(); i++){
        if (!std::isalpha(cipher[i])) {
            plaintext += cipher[i];
            continue;
        }

        // Convert to 0-25 range
        int cipher_val = std::toupper(cipher[i]) - 'A';
        int key_val = std::toupper(key[i % key.size()]) - 'A';

        // Decrypt and wrap around
        int decrypted_val = mod_positive(cipher_val - key_val, 26);
        
        // Convert back to character, preserving original case
        char decrypted_char = decrypted_val + 'A';
        plaintext += std::islower(cipher[i]) ? std::tolower(decrypted_char) : decrypted_char;
    }
    return plaintext;
}

class Table {
    private:
        std::vector<std::vector<std::string>> data;
        int column_width = 15;
    
    public:
        // Add a row to the table
        void addRow(const std::vector<std::string>& row) {
            data.push_back(row);
        }
    
        // Print the table
        void print(std::ostream& out = std::cout) {
            // Check if table is empty
            if (data.empty()) {
                out << "Empty table" << std::endl;
                return;
            }
    
            // Print header separator
            for (size_t i = 0; i < data[0].size(); ++i) {
                out << std::setw(column_width) << std::setfill('-') << "-";
            }
            out << std::endl;
    
            // Print rows
            for (const auto& row : data) {
                for (const auto& cell : row) {
                    out << std::left << std::setw(column_width) << std::setfill(' ') << cell;
                }
                out << std::endl;
            }
    
            // Print footer separator
            for (size_t i = 0; i < data[0].size(); ++i) {
                out << std::setw(column_width) << std::setfill('-') << "-";
            }
            out << std::endl;
        }
    };


int main(int argc, char* argv[]){
    if (argc <= 1){
        perror("invalid args");
    }
    
    std::string filename = argv[1]; //read the input into a string
    std::ifstream inputFile(filename);
    std::ofstream outputFile("output.txt");
    string cipher;
    string line;
    if (inputFile.is_open()) {
        while (std::getline(inputFile, line)) {
            cipher += line + "\n";  // Append each line with a newline
        }
        inputFile.close();
    } else {
        std::cerr << "Unable to open file" << std::endl;
    }

    outputFile << "Part 1:" << std::endl;
    outputFile << "step 1: m = 6" << std::endl; //part 1 step 1
    vector<double> ioc_6 = calculate_ioc(6, cipher);
    for (int i = 0; i < 6; i++){
        outputFile << ioc_6[i];
        outputFile << " ";
    }
    outputFile << std::endl;

    outputFile << "step 2: m = 7" << std::endl; //part 1 step 2
    vector<double> ioc_7 = calculate_ioc(7, cipher);
    for (int i = 0; i < 7; i++){
        outputFile << ioc_7[i];
        outputFile << " ";
    }
    outputFile << std::endl;

    outputFile << "step 3: m = 8" << std::endl; //part 1 step 3
    vector<double> ioc_8 = calculate_ioc(8, cipher);
    for (int i = 0; i < 8; i++){
        outputFile << ioc_8[i];
        outputFile << " ";
    }
    outputFile << std::endl;
    outputFile << "step 4: " << std::endl; //type answer for part 4 manually

    outputFile << "\nPart 2: " << std::endl;
    Table analysis;
    vector<vector<double>> m_g = crack_key(7, build_substring_arr(7, cipher)); //make the 2d array

    analysis.addRow({"g", "Mg_1", "Mg_2", "Mg_3", "Mg_4", "Mg_5", "Mg_6", "Mg_7"});
    for (int i = 0; i < 26; i++){ //add each row to the table
        vector<string> row;
        row.push_back(std::to_string(i));
        row.push_back(std::to_string(m_g[i][0]));
        row.push_back(std::to_string(m_g[i][1]));
        row.push_back(std::to_string(m_g[i][2]));
        row.push_back(std::to_string(m_g[i][3]));
        row.push_back(std::to_string(m_g[i][4]));
        row.push_back(std::to_string(m_g[i][5]));
        row.push_back(std::to_string(m_g[i][6]));
        analysis.addRow(row);
    }
    analysis.print(outputFile); //print the table to the output file
    string key = find_key(m_g, 7);
    outputFile << key << std::endl; //print the key to the output file

    outputFile << "\nPart 3:" << std::endl;
    outputFile << decrypt(cipher, key);

    return 0;
}
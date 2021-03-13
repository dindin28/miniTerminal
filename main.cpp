#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <vector>

#include <unistd.h>
#include <termios.h>

char getch(void)
{
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if(tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if(read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    printf("%c\n", buf);
    return buf;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

struct Orders{
    float buyPrice, total;
    std::string symbol;
};

class Parser{
private:
    struct jsonPair{
        std::string key, string_value;
        int value = 0;
        bool isInt;
    };
    std::vector<jsonPair> node;
public:
    Parser(std::string nameOfFile){
        node.clear();
        std::ifstream fin(nameOfFile);
        std::string stringToParse;
        if(fin){
            fin.seekg(0, fin.end);
            int length = fin.tellg();
            fin.seekg(0, fin.beg);
            stringToParse.resize(length);
            for(int i = 0; i <= length; i++){
                stringToParse[i] = fin.get();
            }
            int currentPos = stringToParse.find("{");
            currentPos = stringToParse.find("\"", currentPos);
            while(currentPos != -1){
                jsonPair jsonBuffer;
                int nextPos = stringToParse.find("\"", currentPos + 1);
                for(int i = currentPos + 1; i < nextPos; i++){
                    jsonBuffer.key += stringToParse[i];
                }
                if(stringToParse[nextPos + 2] != '\"'){
                    jsonBuffer.isInt == true;
                    currentPos = nextPos + 2;
                    nextPos = stringToParse.find(",", currentPos);
                    std::string buff;
                    for(int i = currentPos; i < nextPos; i++){
                        buff += stringToParse[i];
                    }
                    jsonBuffer.value = std::stoi(buff);
                }else{
                    jsonBuffer.isInt = false;
                    currentPos = (nextPos + 3);
                    nextPos = stringToParse.find("\"", currentPos);
                    for(int i = currentPos; i < nextPos; i++){
                        jsonBuffer.string_value += stringToParse[i];
                    }
                }
                node.push_back(jsonBuffer);
                currentPos = stringToParse.find("\"", nextPos + 1);
            }
        }else{
            std::cout << "No file with name " << nameOfFile << " found" << std::endl;
        }
    }
    std::string getValue(std::string nameOfKey){
        for(int i = 0; i < node.size(); i++){
            if(node[i].key == nameOfKey){
                if(node[i].isInt == true){
                    return std::to_string(node[i].value);
                }else{
                    return node[i].string_value;
                }
            }
        }
        return "";
    }
};

void showVector(std::vector<Orders> &v){
    std::cout << "AvrPrice total (symbol)" << std::endl;
    for(int i = 0; i < v.size(); i++){
        std::cout << i + 1 << ") " << v[i].buyPrice << " " << v[i].total << "(" << v[i].symbol << ")" << std::endl;
    }
}

int main()
{
    std::ifstream f("data.txt");
    if(f.is_open()){
        std::vector<Orders> orders;
        Orders buffOrder;
        f.seekg(0, f.end);
        int endOfFile = f.tellg();
        f.seekg(0, f.beg);
        while(static_cast<int>(f.tellg()) + 2 != endOfFile){
            f >> buffOrder.buyPrice;
            f >> buffOrder.total;
            f >> buffOrder.symbol;
            orders.push_back(buffOrder);
        }
        int ch;
        do{
            do {
                system("clear");
                showVector(orders);
                std::cout << std::endl;
                std::cout << "Choose option: " << std::endl;
                std::cout << "1) Enter new order" << std::endl;
                std::cout << "2) Delete order" << std::endl;
                std::cout << "3) Swap orders" << std::endl;
                std::cout << "4) Show percentage: " << std::endl;
                std::cout << "0) Exit" << std::endl;
                std::cin >> ch;
            }while(ch != 1 && ch != 2 && ch != 3 && ch != 4 && ch != 0);
            switch(ch){
                case(1):{
                    std::cout << "Enter average price: ";
                    std::cin >> buffOrder.buyPrice;
                    std::cout << "\nEnter total: ";
                    std::cin >> buffOrder.total;
                    std::cout << "\nEnter symbol: ";
                    std::cin >> buffOrder.symbol;
                    orders.push_back(buffOrder);
                    break;
                }
                case(2):{
                    if(orders.size() != 0){
                        int k;
                        do{
                            system("clear");
                            showVector(orders);
                            std::cout << std::endl;
                            std::cout << "0) Exit" << std::endl;
                            std::cout << "Which order do you want to erase: ";
                            std::cin >> k;
                        }while(k < 1 || k > orders.size());
                        if(k != 0) {
                            orders.erase(orders.begin() + k - 1);
                        }
                        ch = -1;
                    }
                    break;
                }
                case(3):{
                    if(orders.size() > 1){
                        int k, j;
                        do{
                            system("clear");
                            showVector(orders);
                            std::cout << std::endl;
                            std::cout << "0) Exit" << std::endl;
                            std::cout << "Which orders do you want to swap: ";
                            std::cin >> k >> j;
                        }while((k < 0 || k > orders.size()) && (j < 0 || j > orders.size()));
                        if(k != 0 && j != 0){
                            buffOrder = orders[k - 1];
                            orders[k - 1] = orders[j - 1];
                            orders[j - 1] = buffOrder;
                        }
                    }
                    break;
                }
                case(4):{
                    std::cout << "AvrPrice total (symbol): income($), income(%)" << std::endl;
                    for(int i = 0; i < orders.size(); i++){
                        curl_global_init(CURL_GLOBAL_ALL);
                        CURL *curl_handle = curl_easy_init();
                        FILE *pagefile = fopen("temporaryData.txt", "wb");
                        curl_easy_setopt(curl_handle, CURLOPT_HEADER, "X-MBX-APIKEY: dimashtrikker");
                        std::string str = "https://api.binance.com/api/v3/avgPrice?symbol=" + orders[i].symbol;
                        char *head = (char *) malloc(str.size());
                        for(int i = 0; i < str.size(); i++){
                            *(head + i) = str[i];
                        }
                        curl_easy_setopt(curl_handle, CURLOPT_URL, head);
                        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
                        if(pagefile) {
                            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
                            curl_easy_perform(curl_handle);
                            fclose(pagefile);
                        }
                        curl_easy_cleanup(curl_handle);
                        curl_global_cleanup();
                        free(head);
                        Parser parser("temporaryData.txt");
                        float incomePer = std::stof(parser.getValue("price")) / orders[i].buyPrice;
                        std::cout << i + 1 << ") " << orders[i].buyPrice << " " << orders[i].total << "(" << orders[i].symbol << "): " << orders[i].total * incomePer - orders[i].total << "$, " << incomePer << "%" << std::endl;
                    }
                    getch();
                }
            }
        }while(ch != 0);
        f.close();
        std::ofstream fout("data.txt");
        for(int i = 0; i < orders.size(); i++){
            fout << orders[i].buyPrice << " " << orders[i].total << " " << orders[i].symbol << "\r\n";
        }
    }else{
        std::cout << "No file found!" << std::endl;
    }
    return 0;
}
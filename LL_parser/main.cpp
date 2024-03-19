#include <bits/stdc++.h>

using namespace std;

vector<string> splitter(const string &str);

void lexical(const vector<string> &v);

class Parser {
private:
    map<string, double> ID_name;
    int error_code = 0, putNAN = 0;
    string error_str;
public:
    void program(const vector<string> &v) {
        error_code = 0; // error 발생을 대비한 값 0이 아니면 코드 에러 발생을 의미
        error_str = ""; // error 메시지 저장할 멤버 string
        putNAN = 0;
        statements(v);
    }

    void statements(const vector<string> &v) {
        vector<string> tmp1; // ; 앞의 한 문장을 임시로 저장
        vector<string> tmp2; // ; 뒤의 전체 문장을 임시로 저장 *사실상 비어있는 벡터(한줄씩 입력받기 때문)
        int state = 0;       // statements 특정 값이 식별 되었을 때 분기를 위한 상태 변수
        for (const string &i: v) {
            if (i == ";") {
                if (state == 1) tmp2.push_back(i); // 한 번 ";" 등장한 이후 ;는 tmp2에 임시 저장
                else state = 1; // 처음 한 번 등장한 ";"만 의미가 있어서 상태 변수 값을 1로 만들어 분기
            } else if (state == 0) tmp1.push_back(i); // ";" 나오기 전까지 앞의 안 문장을 push_back
            else tmp2.push_back(i); // state 1이고 ; 나오지 않은 상황
        }
        if (!v.empty()) {
            statement(tmp1);
            statements(tmp2);
        }
    }

    void statement(const vector<string> &v) {
        vector<string> tmp1; // := 앞 L-value
        vector<string> tmp2; // := 뒤 R-value
        int state = 0;
        for (const string &i: v) {
            if (i == ":=") state = 1; // 등장 했을 때 따로 임시 vector에 넣지는 않고 상태변수 값 1로 변경
            else if (state == 0) tmp1.push_back(i); // L-value tmp1에 차례로 push_back
            else tmp2.push_back(i); // R-value tmp2에 차례로 push_back
        }
        double exp = expresssion(tmp2); // R-value이므로 결국 int 값으로 리턴받음
        if (putNAN) {
            ID_name[tmp1.front()] = NAN;
        } else {
            ID_name[tmp1.front()] = exp; //L-value: 하나의 key 값을 묶어둠.(즉 tmp1은 ID이고 exp는 R-value)
        }
    }

    double expresssion(const vector<string> &v) {
        vector<string> tmp1; // + or - 가 나오기 전의 term을 저장할 임시 벡터
        vector<string> tmp2; // + or - 가 나온 후의 term_tail 저장할 임시 벡터(+,- 포함)
        int state = 0;
        for (const string &i: v) {
            if (i == "+" || i == "-") { // + or - 식별되면 상태 변수 : 1 하고 이 연산자를 tmp2에 push_back
                state = 1;
                tmp2.push_back(i); // 사실 + -는 무조건 termtail 이 받음
            } else if (state == 0) tmp1.push_back(i); // + or - 나오기 전 어떠한 value(variable 또는 constant)
            else tmp2.push_back(i); // + or - 한 번이라도 나온 뒤 termtail
        }
        return term(tmp1) + term_tail(tmp2); // 결국 계산될 R-value를 int exp = expresssion(tmp2); 이유로 return
    }

    double term_tail(const vector<string> &v) {
        vector<string> tmp1;
        vector<string> tmp2;
        int state = 0;
        for (int i = 0; i < v.size(); i++) {
            if (i) {
                if (i == 1 && isoperator(v[i])) {
                    error_code = max(error_code, 1);
                    error_str += "# 중복 연산자(" + v[i] + ") 제거\t";
                } else {
                    if (state) tmp2.push_back(v[i]);
                    else if (v[i] == "+" || v[i] == "-") {
                        state = 1;
                        tmp2.push_back(v[i]);
                    } else tmp1.push_back(v[i]);
                }
            }
        }
        if (v.empty()) return 0;
        else if (v.front() == "+") return term(tmp1) + term_tail(tmp2);
        else return term(tmp1) - term_tail(tmp2);
    }

    double term(const vector<string> &v) {
        vector<string> tmp1; // term은 내부적으로 더 하위 노드에서 (* or /)가 계산되어야 해서 * / 나오기 전  factor로 넘김
        vector<string> tmp2; // * or / 한 번 등장 이후의 모든 factor들의 결합 넘김
        int state = 0;
        for (const string &i: v) {
            if (i == "*" || i == "/") {
                state = 1;
                tmp2.push_back(i);
            } else if (state == 0) tmp1.push_back(i);
            else tmp2.push_back(i);
        }
        return factor(tmp1) * factor_tail(tmp2);
    }

    double factor_tail(const vector<string> &v) {
        vector<string> tmp1;
        vector<string> tmp2;
        int state = 0;
        for (int i = 0; i < v.size(); i++) {
            if (i) {
                if (i == 1 && isoperator(v[i])) {
                    error_code = max(error_code, 1);
                    error_str += "# 중복 연산자(" + v[i] + ") 제거\t";
                } else {
                    if (state) tmp2.push_back(v[i]);
                    else if (v[i] == "*" || v[i] == "/") {
                        state = 1;
                        tmp2.push_back(v[i]);
                    } else tmp1.push_back(v[i]);
                }
            }
        }
        if (v.empty()) {
            return 1;
        } else {
            double a = factor(tmp1);
            double b = factor_tail(tmp2);
            if (b != 0) {
                if (v.front() == "*") return a * b;
                else return 1.0 / a * b;
            } else return a;

        }
    }

    double factor(const vector<string> &v) {
        vector<string> tmp1;
        int state = 0;
        if (v.size() == 1) {
            string str = v.front();
            if (str[0] == '+') {
                str = str.substr(1);
            } else if (str[0] == '-') {
                str = str.substr(1);
                state = 1;
            }

            if (str[0] <= '9' && str[0] >= '0') {
                if (state) return -stod(str);
                else return stod(str);
            } else {
                if (ID_name.find(str) != ID_name.end()) {
                    if (ID_name[str] == NAN) {
                        putNAN = 1;
                        return ID_name[str];
                    } else {
                        if (state) return -ID_name[str];
                        else return ID_name[str];
                    }
                } else {
                    error_code = max(error_code, 2);
                    putNAN = 1;
                    error_str += "# 정의되지 않은 변수(" + str + ")가 참조됨\t";
                    ID_name[str] = NAN;
                    return 0;
                }
            }
        } else {
            for (const auto &i: v) if (i != "(" && i != ")") tmp1.push_back(i);
            return expresssion(tmp1);
        }
    };

    void print_result() const {
        if (error_code <= 0) cout << "(OK)";
        else cout << "(Warning)  " << error_str;
    }

    void print_ID() {
        cout << "Result ==>";
        for (const auto &i: ID_name) {
            if (i.second != NAN) cout << " " << i.first << ": " << i.second << ";";
            else cout << " " << i.first << ": Unknown;";
        }
    }

    bool isoperator(const string &str) {
        char arr[4][2] = {"+", "-", "*", "/"};
        for (auto &i: arr)
            if (str == i) return true;
        return false;
    }
};


vector<string> splitter(const string &str) {
    vector<string> result;
    istringstream iss(str);
    string word;
    while (iss >> word) {
        size_t pos = 0;
        while ((pos = word.find('(', pos)) != string::npos) {
            if (pos != 0) {
                result.push_back(word.substr(0, pos));
                word = word.substr(pos);
                pos = 0;
            } else {
                if (word.size() > 1) {
                    result.emplace_back("(");
                    word = word.substr(1);
                } else {
                    result.push_back(word);
                    word = "";
                    break;
                }
            }
        }
        pos = 0;
        while ((pos = word.find(')', pos)) != string::npos) {
            if (pos != 0) {
                result.push_back(word.substr(0, pos));
                word = word.substr(pos);
                pos = 0;
            } else {
                if (word.size() > 1) {
                    result.emplace_back(")");
                    word = word.substr(1);
                } else {
                    result.push_back(word);
                    word = "";
                    break;
                }
            }
        }
        if (!word.empty()) result.push_back(word);
    }
    return result;
}

void lexical(const vector<string> &v) {
    set<string> ID;
    vector<string> CONST;
    vector<string> OP;
    for (auto i: v) {
        if (i == "*" || i == "-" || i == "+" || i == "/") {
            OP.push_back(i);
        } else {
            if (i[0] == '+') {
                i = i.substr(1);
            } else if (i[0] == '-') {
                i = i.substr(1);
            }
            if (i[0] <= '9' && i[0] >= '0') {
                CONST.push_back(i);
            } else if ((i[0] >= 'A' && i[0] <= 'Z') || (i[0] >= 'a' && i[0] <= 'z')) {
                ID.insert(i);
            }
        }
    }
    cout << "ID: " << ID.size() << "; CONST: " << CONST.size() << "; OP: " << OP.size() << ";" << endl;
}

int main() {
    string fname, str;
    cin >> fname;
    ifstream ifs(fname);
    Parser parser;
    while (getline(ifs, str)) {
        cout << str << endl;
        vector<string> lineinf = splitter(str);
        lexical(lineinf);
        parser.program(lineinf);
        parser.print_result();
        cout << endl << endl;
    }
    parser.print_ID();
    cout << endl;
    return 0;
}
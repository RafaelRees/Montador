#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

string trim(const string& str) {
  size_t first = str.find_first_not_of(' ');
  if (string::npos == first) {
    return str;
  }
  size_t last = str.find_last_not_of(' ');
  return str.substr(first, (last - first + 1));
}

vector<string> split_string(const string& str, char split_point) {
  auto result = vector<string>{};
  auto ss = stringstream{str};

  for (string line; getline(ss, line, split_point);) {
    line = trim(line);
    result.push_back(line);
  }

  return result;
}

bool eh_palavra_reservada(string palavra) {
  return palavra == "push" || palavra == "add" || palavra == "sub" ||
         palavra == "mul" || palavra == "div" || palavra == "store" ||
         palavra == "jmp" || palavra == "jeq" || palavra == "jgt" ||
         palavra == "jlt" || palavra == "in" || palavra == "out" ||
         palavra == "stp" || palavra == "pop";
}

void valida_nome_simbolo(string nome) {
  if (eh_palavra_reservada(nome)) {
    cout << "nome de label \"" << nome
         << "\" invalido, pois eh palavra reservada" << endl;
  }
}

vector<string> ordena_declaracoes(vector<string> linhas) {
  vector<string> linhas_normais;
  vector<string> linhas_space;

  for (auto linha : linhas) {
    auto partes = split_string(linha, ' ');
    if (partes.size() == 3) {
      auto instrucao = partes[1];
      if (instrucao == "space") {
        linhas_space.push_back(linha);
      } else {
        linhas_normais.push_back(linha);
      }

    } else if (partes.size() == 2 && eh_palavra_reservada(partes[1])) {
      auto instrucao = partes[0];
      if (instrucao == "space") {
        linhas_space.push_back(linha);
      } else {
        linhas_normais.push_back(linha);
      }
    } else {
      linhas_normais.push_back(linha);
    }
  }

  // Concatenar linhas com o space ao final das linhas com instruções normais
  linhas_normais.insert(
    linhas_normais.end(),
    linhas_space.begin(),
    linhas_space.end()
  );

  return linhas_normais;
}

// Percorre o conteúdo procurando linhas do tipo "A B C", onde A são as labels
map<string, int> gera_tabela_de_simbolos(vector<string> conteudo) {
  auto result = map<string, int>{};
  int i = 0;

  for (auto linha : conteudo) {
    auto partes = split_string(linha, ' ');
    if (partes.size() == 3) {
      result[partes[0]] = i;
      valida_nome_simbolo(partes[0]);

    } else if (partes.size() == 2 && eh_palavra_reservada(partes[1])) {
      result[partes[0]] = i;
      valida_nome_simbolo(partes[0]);
    }

    i++;
  }

  return result;
}

int resolve_simbolo(string simbolo, map<string, int> simbolos) {
  if (simbolos.find(simbolo) != simbolos.end()) {
    return simbolos[simbolo];
  } else {
    try {
      return std::stoi(simbolo);
    } catch (const exception& e) {
      cout << "esperava um numero, encontrei \"" << simbolo
           << "\" (talvez voce tenha mencionado uma label que nao existe?)"
           << endl;
      exit(1);
    }
  }
}

string codificar_operacao_e_operando(string operacao, string operando,
                                     map<string, int> simbolos) {
  if (operacao == "push") {
    return "00 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "add") {
    return "01 00";
  } else if (operacao == "sub") {
    return "02 00";
  } else if (operacao == "mul") {
    return "03 00";
  } else if (operacao == "div") {
    return "04 00";
  } else if (operacao == "store" || operacao == "pop") {
    return "05 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "jmp") {
    return "06 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "jeq") {
    return "07 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "jgt") {
    return "08 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "jlt") {
    return "09 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "in") {
    return "10 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "out") {
    return "11 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "stp") {
    return "12 00";
  } else if (operacao == "space") {
    return "99 " + to_string(resolve_simbolo(operando, simbolos));
  } else {
    cout << "erro de sintaxe, esperava instrucao, encontrei \"" << operacao
         << "\"" << endl;
    exit(1);
  }
}

vector<string> monta_programa(vector<string> linhas,
                              map<string, int> simbolos) {
  auto result = vector<string>{};
  int i = 0;

  for (auto linha : linhas) {
    auto partes = split_string(linha, ' ');
    string instrucao;

    switch (partes.size()) {
      case 1:
        instrucao = codificar_operacao_e_operando(partes[0], "", simbolos);
        break;
      case 2:
        if (eh_palavra_reservada(partes[1])) {
          instrucao = codificar_operacao_e_operando(partes[1], "", simbolos);
        } else {
          instrucao =
              codificar_operacao_e_operando(partes[0], partes[1], simbolos);
        }
        break;
      case 3:
        instrucao =
            codificar_operacao_e_operando(partes[1], partes[2], simbolos);
        break;
      default:
        cout << "erro de sintaxe na linha " << i + 1
             << ". Esperava uma instrução, encontrei \"" << linha << "\""
             << endl;
        exit(1);
        break;
    }
    result.push_back(instrucao);
    i++;
  }

  return result;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "necessario o argumento de nome do arquivo!" << endl;
    return 1;
  }

  std::ifstream ifs(argv[1]);
  std::string conteudo((std::istreambuf_iterator<char>(ifs)),
                       (std::istreambuf_iterator<char>()));

  auto linhas = split_string(conteudo, '\n');
  linhas = ordena_declaracoes(linhas);
  auto simbolos = gera_tabela_de_simbolos(linhas);
  auto programa = monta_programa(linhas, simbolos);

  // Printa a tabela de simbolos
  // for (map<string, int>::const_iterator it = simbolos.begin();
  //      it != simbolos.end(); ++it) {
  //   std::cout << "(" << it->first << ":" << it->second << ")"
  //             << "\n";
  // }

  // Printa o programa
  // for (auto linha : programa) {
  //   cout << linha << endl;
  // }

  std::ofstream outfile("out.mem");
  for (auto linha : programa) {
    outfile << linha << endl;
  }
  outfile.close();

  return 0;
}

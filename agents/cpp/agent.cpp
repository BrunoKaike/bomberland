#include "game_state.hpp"
#include "behaviour_tree.cpp"

#include <string>
#include <stdlib.h>
#include <iostream>
#include <random>
#include <list>

const std::vector<std::string> _actions = {"up", "left", "right", "down", "bomb", "detonate"};

typedef struct{

  int X;
  int Y;
  std::string ent;
  int valor;
  int dist;
  int custo;
  int Parentes[2];

}Coordenadas;

class Agent {
private:
  static std::string my_agent_id;
  static std::string enemy_agent_id;
  static std::vector<std::string> my_units;
  static std::vector<std::string> enemy_units;
  static Coordenadas mapa[15][15];

  static int tick;

  static void on_game_tick(int tick, const json& game_state);
  static int distanciaAbsoluta(int x1,int y1,int x2,int y2);
public:
  static void run();
};

std::string Agent::my_agent_id;
std::string Agent::enemy_agent_id;
std::vector<std::string> Agent::my_units;
std::vector<std::string> Agent::enemy_units;
Coordenadas Agent::mapa[15][15];

std::vector<Coordenadas> OPEN, CLOSED, CAMINHO;
Coordenadas Start,Goal,Actual,Movement;

int Agent::tick;
int tempo;
std::string action;



bool bombaOk(int X, int Y, Coordenadas mapa[15][15]){

  if(mapa[Y-1][X-1].ent == "v"){
    return true;
  }
  else if(mapa[Y-1][X+1].ent == "v"){
    return true;
  }
  else if(mapa[Y+1][X-1].ent == "v"){
    return true;
  }
  else if(mapa[Y+1][X+1].ent == "v"){
    return true;
  }
  else{
    return false;
  }
  
}

void Agent::run() 
{
  const char* connection_string = getenv ("GAME_CONNECTION_STRING");

  if (connection_string == NULL) 
  {
    connection_string = "ws://127.0.0.1:3000/?role=agent&agentId=agentId&name=defaultName";
  }
  std::cout << "The current connection_string is: " << connection_string << std::endl;

  GameState::connect(connection_string); 
  GameState::set_game_tick_callback(on_game_tick);
  GameState::handle_messages();
}

void Agent::on_game_tick(int tick_nr, const json& game_state) 
{
  DEBUG("*************** on_game_tick");
  DEBUG(game_state.dump());
  TEST(tick_nr, game_state.dump());
  
  tick = tick_nr;
  std::cout << "Tick #" << tick << std::endl;
  if (tick == 1)
  {
    my_agent_id = game_state["connection"]["agent_id"];
    std::cout << my_agent_id << std::endl;
    my_units = game_state["agents"][my_agent_id]["unit_ids"].get<std::vector<std::string>>();
    if(my_agent_id == "a"){
      enemy_agent_id = "b";
      enemy_units = game_state["agents"][enemy_agent_id]["unit_ids"].get<std::vector<std::string>>();
    }
    else{
      enemy_agent_id = "a";
      enemy_units = game_state["agents"][enemy_agent_id]["unit_ids"].get<std::vector<std::string>>();
    }
  }

  std::vector<int> My_Cords = game_state["unit_state"][my_units[0]]["coordinates"].get<std::vector<int>>();
  My_Cords[1] = (My_Cords[1]-14)*-1;

  std::cout << "X: "<< My_Cords[0] << "Y: "<< My_Cords[1]<< std::endl;

  std::vector<int> Enemy_Cords = game_state["unit_state"][enemy_units[0]]["coordinates"].get<std::vector<int>>();
  Enemy_Cords[1] = (Enemy_Cords[1]-14)*-1;

  std::cout << "X: "<< Enemy_Cords[0] << "Y: "<< Enemy_Cords[1]<< std::endl;
  
  int maxX = game_state["world"]["width"];
  int maxY = game_state["world"]["height"];
  const json& entidades = game_state["entities"];

  for(int i = 0; i < maxX; i++){
    for(int j = 0; j < maxY; j++){
      mapa[i][j].X = j;
      mapa[i][j].Y = i;
      mapa[i][j].ent = "v";
      mapa[i][j].valor = 0;
    }
  }  

  Estado estadoGame;
  estadoGame.powerUpNoMapa = false;

  for (const auto& entity_string: entidades){
    int x = entity_string["x"];
    int y = entity_string["y"];
    y = (y-14) * -1;
    mapa[y][x].ent = entity_string["type"];

    if(entity_string["type"] == "b"||entity_string["type"] == "x"){
      if(entity_string["type"] == "b"){
        for(int i = 0; i<= (int)entity_string["blast_diameter"]/2;i++){
          if(y-i>=0 && y-i<15){
            mapa[y-i][x].valor = -5;
          }
          if(y+i>=0 && y+i<15){
            mapa[y+i][x].valor = -5;
          }
          if(x-i>=0 && x-i<15){
            mapa[y][x-i].valor = -5;
          }
          if(x+i>=0 && x+i<15){
            mapa[y][x+i].valor = -5;
          }
        }
      }
      mapa[y][x].valor = -5;
    }
    else{
      mapa[y][x].valor = 0;
    }

    if(entity_string["type"] == "w"){
      mapa[y][x].valor = -2;
    }

    if(entity_string["type"] == "o"){
      mapa[y][x].valor = -3;
    }
    if(entity_string["type"] == "m"){
      mapa[y][x].valor = -4;
    }

    if(entity_string["type"] == "bp"||entity_string["type"] == "fp"){
      mapa[y][x].valor = 1;
      estadoGame.powerUpNoMapa = true;
    }
    
  }

  for(size_t i = 0; i< maxX; i++){
    for(size_t j = 0; j< maxY; j++){
      std::cout << mapa[i][j].ent << " ";
    }

    std::cout << " / ";

    for(size_t j = 0; j< maxY; j++){
      if(mapa[i][j].valor>=0){
        std::cout << " " << mapa[i][j].valor << " ";
      }
      else{
        std::cout << mapa[i][j].valor << " ";
      }
    }

    std::cout << std::endl; 
  }
  
  //INICIALIZANDO ESTADOS
  
  estadoGame.estaEmPerigo = (mapa[My_Cords[1]][My_Cords[0]].valor == -5) ? true : false;
  estadoGame.inimigoEmPerigo = (mapa[Enemy_Cords[1]][Enemy_Cords[0]].valor == -5) ? true : false;
  estadoGame.estaVizinho = (distanciaAbsoluta(My_Cords[1], My_Cords[0], Enemy_Cords[1], Enemy_Cords[0])==1) ? true : false;

 

  int duracaoPowerUpGelo = 15;
  
  estadoGame.pegouPowerUpGelo - (mapa[My_Cords[1]][My_Cords[0]].ent == "fp") ? true : false;
  if(estadoGame.pegouPowerUpGelo ==  true){
    tempo = tick + duracaoPowerUpGelo;
  }

  if(tick <= tempo){
    estadoGame.pegouPowerUpGelo = true;
  }

  behaviourTree(estadoGame);

  Start.X = My_Cords[0];
  Start.Y = My_Cords[1];
  Start.custo = mapa[Start.Y][Start.X].valor;
  Start.ent = mapa[Start.Y][Start.X].ent;
  Start.dist = 0; //H
  Start.Parentes[0] = -1, Start.Parentes[1] = -1;

  Movement = Start;

  Goal.X = Enemy_Cords[0]+1;
  Goal.Y = Enemy_Cords[1];
  Goal.valor = mapa[Goal.Y][Goal.X].valor;

  if(Start.X == Goal.X && Start.Y == Goal.Y){
    std::cout<<"CHEGUEI!!!"<<std::endl;
  }

  else{

    OPEN.clear();
    CLOSED.clear();
    CAMINHO.clear();

    OPEN.push_back(Start);

    bool stop;
    
    do{
      int index = 0;
      int min = abs(OPEN[0].valor)+OPEN[0].dist;
      //std::cout<<"OP"<<"X:"<<OPEN[0].X<<" Y: "<<OPEN[0].Y<<std::endl;
      Actual = OPEN[0];

      for(int i=0; i < OPEN.size();i++){
        if(abs(OPEN[i].valor)+OPEN[i].dist < min){
          //std::cout<<"MUDEI: "<<"Actual - X:"<<Actual.X<<" Y: "<<Actual.Y<<" valor: "<<Actual.valor<<" ent: "<<Actual.ent<<std::endl;
          Actual = OPEN[i];
          index = i;
          min = abs(OPEN[i].valor)+OPEN[i].dist;
        }
      }
      
        OPEN.erase(OPEN.begin()+index);

      if(Actual.X == Goal.X && Actual.Y == Goal.Y){
        std::cout<<"ACHEI!!!"<<std::endl;
        CAMINHO.push_back(Actual);

        do{
          for(int j=0;j<CLOSED.size();j++){
              if(Actual.Parentes[0] == CLOSED[j].X && Actual.Parentes[1] == CLOSED[j].Y){
                Actual = CLOSED[j];
              }
            }

            CAMINHO.push_back(Actual);

        }while(Actual.Parentes[0]!=-1 && Actual.Parentes[1]!=-1);
          
        std::cout<<"Movement!"<<std::endl;
        Movement = CAMINHO[CAMINHO.size()-2];
        stop = true;
        break;
      
      }
      else{
        int cond = 0;
        Coordenadas 
        cima = mapa[Actual.Y-1][Actual.X],
        baixo = mapa[Actual.Y+1][Actual.X],
        esq = mapa[Actual.Y][Actual.X-1],
        dir = mapa[Actual.Y][Actual.X+1];


        std::cout<<"Actual - X:"<<Actual.X<<" Y: "<<Actual.Y<<" valor: "<<Actual.valor<<" ent: "<<Actual.ent <<std::endl;
        std::cout<<"cima - X:"<<cima.X<<" Y: "<<cima.Y<<" valor: "<<cima.valor<<" ent: "<<cima.ent << cima.dist  <<std::endl;
        std::cout<<"baix - X:"<<baixo.X<<" Y: "<<baixo.Y<<" valor: "<<baixo.valor<<" ent: "<<baixo.ent << baixo.dist<<std::endl;
        std::cout<<"esq - X:"<<esq.X<<" Y: "<<esq.Y<<" valor: "<<esq.valor<<" ent: "<<esq.ent<<std::endl;
        std::cout<<"dir - X:"<<dir.X<<" Y: "<<dir.Y<<" valor: "<<dir.valor<<" ent: "<<dir.ent <<std::endl;
        if(cima.ent == "v" && cima.valor >= -3){
          for(int i=0;i<CLOSED.size();i++){
            if (cima.X == CLOSED[i].X && cima.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            std::cout<<"CIMA ENTROU"<<std::endl;
            cima.Parentes[0] = Actual.X;
            cima.Parentes[1] = Actual.Y;
            cima.custo = abs(Actual.valor) + 1; //G
            cima.dist = abs(cima.X - Goal.X) + abs(cima.Y - Goal.Y); //H
            std::cout<<"cima HEURISTICA"<< cima.dist  <<std::endl;
            OPEN.push_back(cima);
            //std::cout<<"OP"<<"X:"<<OPEN[0].X<<" Y: "<<OPEN[0].Y<<std::endl;
            //std::cout<<"OP"<<"X:"<<cima.X<<" Y: "<<cima.Y<<std::endl;

          }
        }
        cond = 0;
        if(baixo.ent == "v" && baixo.valor >= -3){
          for(int i=0;i<CLOSED.size();i++){
            if (baixo.X == CLOSED[i].X && baixo.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            std::cout<<"BAIXO ENTROU"<<std::endl;
            baixo.Parentes[0] = Actual.X;
            baixo.Parentes[1] = Actual.Y;
            baixo.custo = abs(Actual.valor)+1; //G
            baixo.dist = abs(baixo.X - Goal.X) + abs(baixo.Y - Goal.Y); //H
            std::cout<<"baixo HEURISTICA"<< baixo.dist  <<std::endl;
            OPEN.push_back(baixo);
          }
        }
        
        cond = 0;
        if(esq.ent == "v" && esq.valor >= -3){
          for(int i=0;i<CLOSED.size();i++){
            if (esq.X == CLOSED[i].X && esq.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            std::cout<<"ESQUERDA ENTROU"<<std::endl;
            esq.Parentes[0] = Actual.X;
            esq.Parentes[1] = Actual.Y;
            esq.custo = abs(Actual.valor)+1; //G
            esq.dist = abs(esq.X - Goal.X) + abs(esq.Y - Goal.Y); //H
            OPEN.push_back(esq);
          }
          
        }
        cond = 0;
        if(dir.ent == "v" && dir.valor >= -3){
          for(int i=0;i<CLOSED.size();i++){
            if (dir.X == CLOSED[i].X && dir.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond!=1){
            std::cout<<"DIREITA ENTROU"<<std::endl;
            dir.Parentes[0] = Actual.X;
            dir.Parentes[1] = Actual.Y;
            dir.custo = Actual.valor; //G
            dir.dist = abs(dir.X - Goal.X) + abs(dir.Y - Goal.Y); //H
            OPEN.push_back(dir);
          }
        }
        CLOSED.push_back(Actual);
      }
      std::cout<<"TAMANHO DO OPEN : "<<OPEN.size()<<std::endl;
      std::cout<<"TAMANHO DO CLOSED : "<<CLOSED.size()<<std::endl;
    }while(!OPEN.empty());
    std::cout<<"SAÍ DO WHILE / "<<Movement.X<<" "<<Movement.Y<<std::endl;
  }
  
  std::cout<<"XM"<<Movement.X <<"YM"<<Movement.Y <<"XMC"<<My_Cords[0] <<"YMC"<<My_Cords[1]<<std::endl;

  if(Movement.Y == My_Cords[1]-1){
    action = "up";
  }

  else if(Movement.Y == My_Cords[1]+1){
    action = "down";
  }

  else if(Movement.X == My_Cords[0]-1){
    action = "left";
  }

  else if(Movement.X == My_Cords[0]+1){
    action = "right";
  }
  

  srand(1234567 * (my_agent_id == "a" ? 1 : 0) + tick * 100 + 13);

  // send each (alive) unit a random action
  for (const auto unit_id: my_units)
  {
    const json& unit = game_state["unit_state"][unit_id];
    if (unit["hp"] <= 0) continue;
    //action = _actions[rand() % _actions.size()];
    std::cout << "action: " << unit_id << " -> " << action << std::endl;

    if (action == "up" || action == "left" || action == "right" || action == "down")
    {
      GameState::send_move(action, unit_id);
    }
    else if (action == "bomb")
    {
      GameState::send_bomb(unit_id);
    }
    else if (action == "detonate")
    {
      const json& entities = game_state["entities"];
      for (const auto& entity: entities) 
      {
        if (entity["type"] == "b" && entity["unit_id"] == unit_id) 
        {
          int x = entity["x"], y = entity["y"];
          GameState::send_detonate(x, y, unit_id);
          break;
        }
      }
    }
    else 
    {
      std::cerr << "Unhandled action: " << action << " for unit " << unit_id << std::endl;
    }
  }
}

int Agent::distanciaAbsoluta(int x1,int y1,int x2,int y2) {
  int distanciaX = x1 - x2;
  int distanciaY = y1 - y2;
  return abs(distanciaX) + abs(distanciaY);
}

int main()
{
  Agent::run();
}



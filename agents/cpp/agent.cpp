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
  static Coordenadas A_Estrela(Coordenadas mapa[15][15]);
  static Coordenadas A_Estrela_Fuga(Coordenadas mapa[15][15]);
  static Coordenadas caixa(Coordenadas mapa[15][15]);
  static Coordenadas pedra(Coordenadas mapa[15][15]);
  static Coordenadas power(Coordenadas mapa[15][15]);
  static Coordenadas fuga(Coordenadas mapa[15][15]);

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
int bombas = 0;



/*bool bombaOk(int X, int Y, Coordenadas mapa[15][15]){

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
  
}*/

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
  
  estadoGame.pegouPowerUpGelo = (mapa[My_Cords[1]][My_Cords[0]].ent == "fp") ? true : false;
  if(estadoGame.pegouPowerUpGelo ==  true){
    tempo = tick + duracaoPowerUpGelo;
  }

  if(tick <= tempo){
    estadoGame.pegouPowerUpGelo = true;
  }

  std::string saida = behaviourTree(estadoGame);
  Coordenadas caixas,pedras,fugir;

  Start.X = My_Cords[0];
  Start.Y = My_Cords[1];
  Start.custo = mapa[Start.Y][Start.X].valor;
  Start.ent = mapa[Start.Y][Start.X].ent;
  Start.dist = 0; //H
  Start.Parentes[0] = -1, Start.Parentes[1] = -1;

  Movement = Start;
  action = " ";
  

  if(saida == "posicionarBomba"){

    action = "bomb";
    estadoGame.explodeCaixa = true;
    bombas++;
 
  } else if(saida == "explodirBomba"){

    action = "detonate";
    bombas--;

    for (const auto unit_id: my_units){
      const json& entities = game_state["entities"];
        for (const auto& entity: entities) 
        {
          if (entity["type"] == "b" && entity["unit_id"] == unit_id) 
          {
            int x = entity["x"], y = entity["y"];
            y = (y-14) * -1;
            if(bombas>0){
                if((abs(Enemy_Cords[0]-x)<=(int)entity["blast_diameter"]/2 ||abs(Enemy_Cords[1]-y)<=(int)entity["blast_diameter"]/2
                  || mapa[y-1][x].ent == "w" || mapa[y+1][x].ent == "w" || mapa[y][x-1].ent == "w" || mapa[y][x+1].ent == "w"
                  || mapa[y-1][x].ent == "o" || mapa[y+1][x].ent == "o" || mapa[y][x-1].ent == "0" || mapa[y][x+1].ent == "o") && !estadoGame.estaEmPerigo){
                  GameState::send_detonate(x, y, unit_id);
                  break;
                }
            }
            
          }
        }
    }

  } else if(saida == "fugir"){
    Goal = fuga(mapa);
    Movement = A_Estrela_Fuga(mapa);

  } else if(saida == "quebrarCaixa"){
    Goal = caixa(mapa);
    if(Goal.X == -1 && Goal.Y == -1){
      Goal = pedra(mapa);
      if(Goal.X == -1 && Goal.Y == -1){
        action = " ";
      }
      else{
        Movement = A_Estrela(mapa);
        if(Start.X == Goal.X && Start.Y == Goal.Y){
          action = "bomb";
          estadoGame.explodeCaixa = true;
          bombas++;
        }
      }
    }
    else{
      Movement = A_Estrela(mapa);
      if(Start.X == Goal.X && Start.Y == Goal.Y){
          action = "bomb";
          estadoGame.explodeCaixa = true;
          bombas++;
      }
    }

  } else if(saida == "pegarPowerUp"){
    Goal = power(mapa);
    std::cout<<"ENT: "<<Goal.ent<<std::endl;
    Movement = A_Estrela(mapa);

  } else if(saida == "perseguirInimigo"){
    Goal.X = Enemy_Cords[0], Goal.Y = Enemy_Cords[1];
    Movement = A_Estrela(mapa); 
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

Coordenadas Agent::A_Estrela(Coordenadas mapa[15][15]){

  Coordenadas caminho;

  OPEN.clear();
  CLOSED.clear();
  CAMINHO.clear();

  OPEN.push_back(Start);

    bool stop;
    
    do{
      int index = 0;
      int min = abs(OPEN[0].valor)+OPEN[0].dist;
      Actual = OPEN[0];

      for(int i=0; i < OPEN.size();i++){
        if(abs(OPEN[i].valor)+OPEN[i].dist < min){
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
        caminho = CAMINHO[CAMINHO.size()-2];
        stop = true;
        return caminho;
      
      }
      else{
        int cond = 0;
        Coordenadas cima ,baixo, esq, dir;
        
        if(Actual.Y-1>=0){
          cima = mapa[Actual.Y-1][Actual.X];
        }
        if(Actual.Y+1<15){
          baixo = mapa[Actual.Y+1][Actual.X];
        }
        if(Actual.X-1>=0){
          esq = mapa[Actual.Y][Actual.X-1];
        }
        if(Actual.X+1<15){
          dir = mapa[Actual.Y][Actual.X+1];
        }
        
        
        if((cima.ent == "v" || cima.ent == "fp" || cima.ent == "bp")  && cima.valor >= -3){
          for(int i=0;i<CLOSED.size();i++){
            if (cima.X == CLOSED[i].X && cima.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            cima.Parentes[0] = Actual.X;
            cima.Parentes[1] = Actual.Y;
            cima.custo = abs(Actual.valor) + 1; //G
            cima.dist = abs(cima.X - Goal.X) + abs(cima.Y - Goal.Y); //H
            OPEN.push_back(cima);
            //std::cout<<"OP"<<"X:"<<OPEN[0].X<<" Y: "<<OPEN[0].Y<<std::endl;
            //std::cout<<"OP"<<"X:"<<cima.X<<" Y: "<<cima.Y<<std::endl;
           }
        }
        cond = 0;
        if((baixo.ent == "v" || baixo.ent == "fp" || baixo.ent == "bp") && baixo.valor >= -3){
          for(int i=0;i<CLOSED.size();i++){
            if (baixo.X == CLOSED[i].X && baixo.Y == CLOSED[i].Y){
              cond = 1; 
              break;
           }
          }
          if(cond !=1){
            baixo.Parentes[0] = Actual.X;
              baixo.Parentes[1] = Actual.Y;
             baixo.custo = abs(Actual.valor)+1; //G
             baixo.dist = abs(baixo.X - Goal.X) + abs(baixo.Y - Goal.Y); //H
             OPEN.push_back(baixo);
            }
          }
        
          cond = 0;
          if((esq.ent == "v" || esq.ent == "fp" || esq.ent == "bp") && esq.valor >= -3){
            for(int i=0;i<CLOSED.size();i++){
             if (esq.X == CLOSED[i].X && esq.Y == CLOSED[i].Y){
               cond = 1; 
               break;
              }
            }
            if(cond !=1){
              esq.Parentes[0] = Actual.X;
              esq.Parentes[1] = Actual.Y;
              esq.custo = abs(Actual.valor)+1; //G
             esq.dist = abs(esq.X - Goal.X) + abs(esq.Y - Goal.Y); //H
             OPEN.push_back(esq);
            }
          
          }
          cond = 0;
          if((dir.ent == "v" || dir.ent == "fp" || dir.ent == "bp") && dir.valor >= -3){
            for(int i=0;i<CLOSED.size();i++){
              if (dir.X == CLOSED[i].X && dir.Y == CLOSED[i].Y){
                cond = 1; 
                break;
             }
            }
            if(cond!=1){
              dir.Parentes[0] = Actual.X;
              dir.Parentes[1] = Actual.Y;
              dir.custo = Actual.valor; //G
              dir.dist = abs(dir.X - Goal.X) + abs(dir.Y - Goal.Y); //H
              OPEN.push_back(dir);
            }
          }
          CLOSED.push_back(Actual);
        
      }

    }while(!OPEN.empty());

    return Start;
}

Coordenadas Agent::A_Estrela_Fuga(Coordenadas mapa[15][15]){

  Coordenadas caminho;

  OPEN.clear();
  CLOSED.clear();
  CAMINHO.clear();

  OPEN.push_back(Start);

    bool stop;
    
    do{
      int index = 0;
      int min = abs(OPEN[0].valor)+OPEN[0].dist;
      Actual = OPEN[0];

      for(int i=0; i < OPEN.size();i++){
        if(abs(OPEN[i].valor)+OPEN[i].dist < min){
          Actual = OPEN[i];
          index = i;
          min = abs(OPEN[i].valor)+OPEN[i].dist;
        }
      }
      
        OPEN.erase(OPEN.begin()+index);

      if(Actual.X == Goal.X && Actual.Y == Goal.Y){
        CAMINHO.push_back(Actual);

        do{
          for(int j=0;j<CLOSED.size();j++){
              if(Actual.Parentes[0] == CLOSED[j].X && Actual.Parentes[1] == CLOSED[j].Y){
                Actual = CLOSED[j];
              }
            }

            CAMINHO.push_back(Actual);

        }while(Actual.Parentes[0]!=-1 && Actual.Parentes[1]!=-1);
        caminho = CAMINHO[CAMINHO.size()-2];
        stop = true;
        return caminho;
      
      }
      else{
        int cond = 0;
        Coordenadas cima ,baixo, esq, dir;
        
        if(Actual.Y-1>=0){
          cima = mapa[Actual.Y-1][Actual.X];
        }
        if(Actual.Y+1<15){
          baixo = mapa[Actual.Y+1][Actual.X];
        }
        if(Actual.X-1>=0){
          esq = mapa[Actual.Y][Actual.X-1];
        }
        if(Actual.X+1<15){
          dir = mapa[Actual.Y][Actual.X+1];
        }
        
        if(cima.ent == "v" && cima.valor >= -5){
          for(int i=0;i<CLOSED.size();i++){
            if (cima.X == CLOSED[i].X && cima.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            cima.Parentes[0] = Actual.X;
            cima.Parentes[1] = Actual.Y;
            cima.custo = abs(Actual.valor) + 1; //G
            cima.dist = abs(cima.X - Goal.X) + abs(cima.Y - Goal.Y); //H
            OPEN.push_back(cima);
           }
        }
        cond = 0;
        if(baixo.ent == "v" && baixo.valor >= -5){
          for(int i=0;i<CLOSED.size();i++){
            if (baixo.X == CLOSED[i].X && baixo.Y == CLOSED[i].Y){
              cond = 1; 
              break;
           }
          }
          if(cond !=1){
            baixo.Parentes[0] = Actual.X;
            baixo.Parentes[1] = Actual.Y;
            baixo.custo = abs(Actual.valor)+1; //G
            baixo.dist = abs(baixo.X - Goal.X) + abs(baixo.Y - Goal.Y); //H
            OPEN.push_back(baixo);
            }
          }
        
          cond = 0;
          if(esq.ent == "v" && esq.valor >= -5){
            for(int i=0;i<CLOSED.size();i++){
             if (esq.X == CLOSED[i].X && esq.Y == CLOSED[i].Y){
               cond = 1; 
               break;
              }
            }
            if(cond !=1){
              esq.Parentes[0] = Actual.X;
              esq.Parentes[1] = Actual.Y;
              esq.custo = abs(Actual.valor)+1; //G
             esq.dist = abs(esq.X - Goal.X) + abs(esq.Y - Goal.Y); //H
             OPEN.push_back(esq);
            }
          
          }
          cond = 0;
          if(dir.ent == "v" && dir.valor >= -5){
            for(int i=0;i<CLOSED.size();i++){
              if (dir.X == CLOSED[i].X && dir.Y == CLOSED[i].Y){
                cond = 1; 
                break;
             }
            }
            if(cond!=1){
              dir.Parentes[0] = Actual.X;
              dir.Parentes[1] = Actual.Y;
              dir.custo = Actual.valor; //G
              dir.dist = abs(dir.X - Goal.X) + abs(dir.Y - Goal.Y); //H
              OPEN.push_back(dir);
            }
          }
          CLOSED.push_back(Actual);
        
      }

    }while(!OPEN.empty());

    return Start;
}

Coordenadas Agent::caixa(Coordenadas mapa[15][15]){

  OPEN.clear();
  CLOSED.clear();

  OPEN.push_back(Start);
    
    do{
      Actual = OPEN[0];
      OPEN.erase(OPEN.begin());

        int cond = 0;
        Coordenadas cima ,baixo, esq, dir;
        
        if(Actual.Y-1>=0){
          cima = mapa[Actual.Y-1][Actual.X];
        }
        if(Actual.Y+1<15){
          baixo = mapa[Actual.Y+1][Actual.X];
        }
        if(Actual.X-1>=0){
          esq = mapa[Actual.Y][Actual.X-1];
        }
        if(Actual.X+1<15){
          dir = mapa[Actual.Y][Actual.X+1];
        }
        

        if(cima.ent == "w"){
          return Actual;
        }
        else if(cima.ent == "v"){
          for(int i=0;i<CLOSED.size();i++){
            if (cima.X == CLOSED[i].X && cima.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(cima);
          }
        }
          
        
        cond = 0;
        if(baixo.ent == "w"){
          return Actual;
        }
        else if(baixo.ent == "v"){
          for(int i=0;i<CLOSED.size();i++){
            if (baixo.X == CLOSED[i].X && baixo.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(baixo);
          }
        }
        
        if(esq.ent == "w"){
          return Actual;
        }
        else if(esq.ent == "v"){
          for(int i=0;i<CLOSED.size();i++){
            if (esq.X == CLOSED[i].X && esq.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(esq);
          }
        }
        
        if(dir.ent == "w"){
          return Actual;
        }
        else if(dir.ent == "v"){
          for(int i=0;i<CLOSED.size();i++){
            if (dir.X == CLOSED[i].X && dir.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(dir);
          }
        }
        
        CLOSED.push_back(Actual);

    }while(!OPEN.empty());

    Coordenadas Falha;
    Falha.X = -1, Falha.Y = -1;
    return Falha;
}

Coordenadas Agent::pedra(Coordenadas mapa[15][15]){

  OPEN.clear();
  CLOSED.clear();

  OPEN.push_back(Start);
    
    do{
      Actual = OPEN[0];
      OPEN.erase(OPEN.begin());

        int cond = 0;
        Coordenadas cima ,baixo, esq, dir;
        
        if(Actual.Y-1>=0){
          cima = mapa[Actual.Y-1][Actual.X];
        }
        if(Actual.Y+1<15){
          baixo = mapa[Actual.Y+1][Actual.X];
        }
        if(Actual.X-1>=0){
          esq = mapa[Actual.Y][Actual.X-1];
        }
        if(Actual.X+1<15){
          dir = mapa[Actual.Y][Actual.X+1];
        }
        

        if(cima.ent == "o"){
          return Actual;
        }
        else if(cima.ent == "v"){
          for(int i=0;i<CLOSED.size();i++){
            if (cima.X == CLOSED[i].X && cima.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(cima);
          }
        }
          
        
        cond = 0;
        if(baixo.ent == "o"){
          return Actual;
        }
        else if(baixo.ent == "v"){
          for(int i=0;i<CLOSED.size();i++){
            if (baixo.X == CLOSED[i].X && baixo.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(baixo);
          }
        }
        
        if(esq.ent == "o"){
          return Actual;
        }
        else if(esq.ent == "v"){
          for(int i=0;i<CLOSED.size();i++){
            if (esq.X == CLOSED[i].X && esq.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(esq);
          }
        }
        
        if(dir.ent == "o"){
          return Actual;
        }
        else if(dir.ent == "v"){
          for(int i=0;i<CLOSED.size();i++){
            if (dir.X == CLOSED[i].X && dir.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(dir);
          }
        }
        
        CLOSED.push_back(Actual);

    }while(!OPEN.empty());

    return Start;
}

Coordenadas Agent::power(Coordenadas mapa[15][15]){

  OPEN.clear();
  CLOSED.clear();

  OPEN.push_back(Start);
    
    do{
      Actual = OPEN[0];
      OPEN.erase(OPEN.begin());

        int cond = 0;
        Coordenadas cima ,baixo, esq, dir;
        
        if(Actual.Y-1>=0){
          cima = mapa[Actual.Y-1][Actual.X];
        }
        if(Actual.Y+1<15){
          baixo = mapa[Actual.Y+1][Actual.X];
        }
        if(Actual.X-1>=0){
          esq = mapa[Actual.Y][Actual.X-1];
        }
        if(Actual.X+1<15){
          dir = mapa[Actual.Y][Actual.X+1];
        }
        

        if(cima.ent == "fp" || cima.ent == "bp"){
          return cima;
        }
        else if(cima.ent == "v"){
          for(int i=0;i<CLOSED.size();i++){
            if (cima.X == CLOSED[i].X && cima.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(cima);
          }
        }
        
        cond = 0;
        if(baixo.ent == "fp" || baixo.ent == "bp"){
          return baixo;
        }
        else if(baixo.ent == "v"){
          for(int i=0;i<CLOSED.size();i++){
            if (baixo.X == CLOSED[i].X && baixo.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(baixo);
          }
        }
        
        if(esq.ent == "fp" || esq.ent == "bp"){
          return esq;
        }
        else if(esq.ent == "v"){
          for(int i=0;i<CLOSED.size();i++){
            if (esq.X == CLOSED[i].X && esq.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(esq);
          }
        }
        
        if(dir.ent == "fp" || dir.ent == "bp"){
          return dir;
        }
        else if(dir.ent == "v"){
          for(int i=0;i<CLOSED.size();i++){
            if (dir.X == CLOSED[i].X && dir.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(dir);
          }
        }
        
        CLOSED.push_back(Actual);

    }while(!OPEN.empty());


    return Start;
}

Coordenadas Agent::fuga(Coordenadas mapa[15][15]){

  OPEN.clear();
  CLOSED.clear();

  OPEN.push_back(Start);
    
    do{
      Actual = OPEN[0];
      OPEN.erase(OPEN.begin());

        int cond = 0;
        Coordenadas cima ,baixo, esq, dir;
        
        if(Actual.Y-1>=0){
          cima = mapa[Actual.Y-1][Actual.X];
        }
        if(Actual.Y+1<15){
          baixo = mapa[Actual.Y+1][Actual.X];
        }
        if(Actual.X-1>=0){
          esq = mapa[Actual.Y][Actual.X-1];
        }
        if(Actual.X+1<15){
          dir = mapa[Actual.Y][Actual.X+1];
        }
        

        if(cima.ent == "v" && cima.valor>=0){
          return cima;
        }
        else if(cima.ent == "v" && cima.valor== -5){
          for(int i=0;i<CLOSED.size();i++){
            if (cima.X == CLOSED[i].X && cima.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(cima);
          }
        }
          
        
        cond = 0;
        if(baixo.ent == "v" && baixo.valor >=0){
          return baixo;
        }
        else if(baixo.ent == "v" && baixo.valor == -5){
          for(int i=0;i<CLOSED.size();i++){
            if (baixo.X == CLOSED[i].X && baixo.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(baixo);
          }
        }
        
        if(esq.ent == "v" && esq.valor >=0){
          return esq;
        }
        else if(esq.ent == "v" && esq.valor == -5){
          for(int i=0;i<CLOSED.size();i++){
            if (esq.X == CLOSED[i].X && esq.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(esq);
          }
        }
        
        if(dir.ent == "v" && dir.valor >= 0){
          return dir;
        }
        else if(dir.ent == "v" && dir.valor == -5){
          for(int i=0;i<CLOSED.size();i++){
            if (dir.X == CLOSED[i].X && dir.Y == CLOSED[i].Y){
              cond = 1; 
              break;
            }
          }
          if(cond !=1){
            OPEN.push_back(dir);
          }
        }
        
        CLOSED.push_back(Actual);

    }while(!OPEN.empty());

    return Start;
}
#include "game_state.hpp"

#include <string>
#include <stdlib.h>
#include <iostream>
#include <random>

const std::vector<std::string> _actions = {"up", "left", "right", "down", "bomb", "detonate"};

typedef struct{

  int X;
  int Y;
  std::string ent;
  int Valor;

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
public:
  static void run();
};

std::string Agent::my_agent_id;
std::string Agent::enemy_agent_id;
std::vector<std::string> Agent::my_units;
std::vector<std::string> Agent::enemy_units;
Coordenadas Agent::mapa[15][15];

int Agent::tick;

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
      mapa[i][j].X = i;
      mapa[i][j].Y = j;
      mapa[i][j].ent = "v";
      mapa[i][j].Valor = 0;
    }
  }  


  for (const auto& entity_string: entidades){
    int x = entity_string["x"];
    int y = entity_string["y"];
    y = (y-14) * -1;
    mapa[y][x].ent = entity_string["type"];

    if(entity_string["type"] == "b"||entity_string["type"] == "x"){
      mapa[y][x].Valor = -5;
    }
    else{
      mapa[y][x].Valor = 0;
    }

    if(entity_string["type"] == "w"){
      mapa[y][x].Valor = -2;
    }

    if(entity_string["type"] == "o"){
      mapa[y][x].Valor = -3;
    }
    if(entity_string["type"] == "m"){
      mapa[y][x].Valor = -4;
    }

    if(entity_string["type"] == "bp"||entity_string["type"] == "fp"){
      mapa[y][x].Valor = 1;
    }
    
  }

  mapa[My_Cords[1]][My_Cords[0]].ent = "P"; // remover futuramente
  mapa[Enemy_Cords[1]][Enemy_Cords[0]].ent = "E";
  mapa[Enemy_Cords[1]][Enemy_Cords[0]].Valor = -1;

  for(size_t i = 0; i< maxX; i++){
    for(size_t j = 0; j< maxY; j++){
      std::cout << mapa[i][j].ent << " ";
    }

    std::cout << " / ";

    for(size_t j = 0; j< maxY; j++){
      std::cout << mapa[i][j].Valor << " ";
    }

    std::cout << std::endl; 
  }
  

  srand(1234567 * (my_agent_id == "a" ? 1 : 0) + tick * 100 + 13);

  // send each (alive) unit a random action
  for (const auto unit_id: my_units)
  {
    const json& unit = game_state["unit_state"][unit_id];
    if (unit["hp"] <= 0) continue;
    std::string action = _actions[rand() % _actions.size()];
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

int main()
{
  Agent::run();
}


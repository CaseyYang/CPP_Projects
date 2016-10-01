#include <iostream>
#include <map>
#include <set>
#include <time.h>
#include <stdlib.h>
using namespace std;

enum PlayerType {
	GOD,
	WOLF,
	VILLAGER
};

enum PlayerStatus {
	LIVE,
	DEAD
};

enum RoundResult {
	GOOD_WIN,
	WOLVES_WIN,
	NO_RESULT
};

struct Player {
	int id;
	PlayerType type;
	PlayerStatus status;
	bool isValid;
	bool isPolice;
	Player(int id) {
		this->id = id;
		this->status = PlayerStatus::LIVE;
		this->isPolice = false;
		this->isValid = false;
	}
};

map<int, Player> players = map<int, Player>();
set<int>live_players = set<int>();
set<int> valids = set<int>();
set<int> gods = set<int>();
set<int> wolves = set<int>();
set<int> villagers = set<int>();
set<int> goods = set<int>();
int police_id = -1;
int predictor_id = -1;

set<int>::const_iterator random_pick_from_set(set<int> &source_set) {
	int number = source_set.size();
	set<int>::const_iterator result_iter = source_set.begin();
	if (number > 0) {
		for (int i = (rand() % number); i > 0; --i) {
			result_iter++;
		}
	}
	return result_iter;
}

bool kill_a_player(int player_id) {
	if (players.at(player_id).status == PlayerStatus::LIVE&&live_players.find(player_id) != live_players.end()) {
		players.at(player_id).status = PlayerStatus::DEAD;
		live_players.erase(player_id);
		valids.erase(player_id);
		gods.erase(player_id);
		wolves.erase(player_id);
		villagers.erase(player_id);
		goods.erase(player_id);
		if (player_id == police_id) {
			police_id = -1;
		}
		return true;
	}
	return false;
}

void init_game() {
	players.clear();
	live_players.clear();
	valids.clear();
	gods.clear();
	wolves.clear();
	villagers.clear();
	police_id = -1;
}

void generate_players(int player_number) {
	//generate players
	for (int i = 0; i < player_number; ++i) {
		players.insert(make_pair(i, Player(i)));
		live_players.insert(i);
	}
	set<int> candidates = set<int>(live_players);
	//generate gods
	for (int i = 0; i < 5; ++i) {
		set<int>::const_iterator god_id_iter = random_pick_from_set(candidates);
		if (god_id_iter != candidates.end()) {
			int god_id = *god_id_iter;
			gods.insert(god_id);
			goods.insert(god_id);
			candidates.erase(god_id);
			players.at(god_id).type = PlayerType::GOD;
		}
		else {
			cout << "canadidates is empty during god generation!";
			exit(1);
		}
	}
	//generate wolves
	for (int i = 0; i < 5; ++i) {
		set<int>::const_iterator wolf_id_iter = random_pick_from_set(candidates);
		if (wolf_id_iter != candidates.end()) {
			int wolf_id = *wolf_id_iter;
			wolves.insert(wolf_id);
			candidates.erase(wolf_id);
			players.at(wolf_id).type = PlayerType::WOLF;
		}
		else {
			cout << "canadidates is empty during wolves generation!";
			exit(1);
		}
	}
	//set villagers
	for (set<int>::iterator iter = candidates.begin(); iter != candidates.end(); iter++) {
		villagers.insert(*iter);
		goods.insert(*iter);
		players.at(*iter).type = PlayerType::VILLAGER;
	}
}

void choose_roles() {
	//choose predictor
	set<int>::const_iterator predictor_id_iter = random_pick_from_set(gods);
	predictor_id = *predictor_id_iter;
	set<int> tmp_goods = set<int>(goods);
	tmp_goods.erase(predictor_id);
	//choose the first valid villager
	set<int>::const_iterator valid1_iter = random_pick_from_set(tmp_goods);
	int valid1_id = *valid1_iter;
	tmp_goods.erase(valid1_id);
	//choose the second valid villager
	set<int>::const_iterator valid2_iter = random_pick_from_set(tmp_goods);
	int valid2_id = *valid2_iter;
	players.at(valid1_id).isValid = true;
	players.at(valid2_id).isValid = true;
	valids.insert(valid1_id);
	valids.insert(valid2_id);
}

void pick_police(bool last_police_is_good) {
	if (last_police_is_good) {
		int live_valids_count = 0;
		int live_valid_id = -1;
		for (set<int>::iterator iter = valids.begin(); iter != valids.end(); ++iter) {
			if (players.at(*iter).status == PlayerStatus::LIVE) {
				live_valids_count += 1;
				live_valid_id = *iter;
			}
		}
		//two strategies
		/*switch (live_valids_count) {
		case 0:
		{
			set<int>::const_iterator police_id_iter = random_pick_from_set(live_players);
			police_id = *police_id_iter;
			break;
		}
		case 1:
			police_id = live_valid_id;
			break;
		case 2:
		{
			set<int>::const_iterator police_id_iter = random_pick_from_set(valids);
			police_id = *police_id_iter;
			break;
		}
		}*/
		switch (live_valids_count) {
		case 1:
			police_id = live_valid_id;
			break;
		default:
		{
			set<int>::const_iterator police_id_iter = random_pick_from_set(valids);
			police_id = *police_id_iter;
			break;
		}
		}
	}
	else {
		set<int>::const_iterator police_id_iter = random_pick_from_set(wolves);
		police_id = *police_id_iter;
	}
}

RoundResult judge_result() {
	if (wolves.empty()) {
		return RoundResult::GOOD_WIN;
	}
	else {
		if (gods.empty()) {
			return RoundResult::WOLVES_WIN;
		}
		else {
			return RoundResult::NO_RESULT;
		}
	}
}

int vote(int player_id) {
	int result_id = -1;
	if (players.at(player_id).type == PlayerType::WOLF) {
		set<int>::const_iterator result_id_iter = random_pick_from_set(goods);
		result_id = *result_id_iter;
	}
	else {
		set<int> candidates = set<int>(live_players);
		candidates.erase(player_id);
		for (set<int>::iterator iter = valids.begin(); iter != valids.end(); ++iter) {
			candidates.erase(*iter);
		}
		set<int>::const_iterator result_id_iter = random_pick_from_set(candidates);
		result_id = *result_id_iter;
	}
	return result_id;
}

void pick_one_to_kill() {
	map<int, float> vote_result = map<int, float>();
	for (set<int>::iterator iter = live_players.begin(); iter != live_players.end(); ++iter) {
		int candidate_id = vote(*iter);
		if (vote_result.find(candidate_id) == vote_result.end()) {
			vote_result.insert(make_pair(candidate_id, 0));
		}
		vote_result.at(candidate_id) += 1;
		if ((*iter) == police_id) {
			vote_result.at(candidate_id) += 0.5;
		}
	}
	float max_score = -1;
	int canadidate_id = -1;
	for (map<int, float>::iterator iter = vote_result.begin(); iter != vote_result.end(); ++iter) {
		if (iter->second > max_score) {
			canadidate_id = iter->first;
			max_score = iter->second;
		}
	}
	kill_a_player(canadidate_id);
}

int wolf_kill() {
	set<int>::const_iterator iter = random_pick_from_set(goods);
	int killed_player_id = *iter;
	kill_a_player(killed_player_id);
	return killed_player_id;
}

RoundResult simulator() {
	//init game
	init_game();
	//generate players and its types
	generate_players(15);
	//choose valid villagers and police
	choose_roles();
	//kill the predictor
	kill_a_player(predictor_id);
	//pick a new police
	pick_police(true);
	//continue the game
	while (1) {
		int dead_player_id = wolf_kill();
		if (judge_result() == RoundResult::NO_RESULT) {
			if (police_id == -1) {
				pick_police(!players.at(dead_player_id).type == PlayerType::WOLF);
			}
			pick_one_to_kill();
			if (judge_result() != RoundResult::NO_RESULT) {
				return judge_result();
			}
		}
		else {
			return judge_result();
		}
	}
}

void main() {
	srand((unsigned)time(NULL));
	int count = 0;
	for (int i = 0; i < 1000000; ++i) {
		RoundResult result = simulator();
		if (result == RoundResult::GOOD_WIN) {
			++count;
		}
		//cout << (count + 0.0) / i << endl;
	}
}
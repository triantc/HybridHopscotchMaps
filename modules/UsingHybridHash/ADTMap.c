/////////////////////////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Map μέσω υβριδικού Hash Table
//
/////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "ADTMap.h"
#include "ADTVector.h"

// Οι κόμβοι του map στην υλοποίηση με hash table, μπορούν να είναι σε 2 διαφορετικές καταστάσεις
typedef enum {
	EMPTY, OCCUPIED
} State;

// Το μέγεθος του Hash Table ιδανικά θέλουμε να είναι πρώτος αριθμός σύμφωνα με την θεωρία.
// Η παρακάτω λίστα περιέχει πρώτους οι οποίοι έχουν αποδεδιγμένα καλή συμπεριφορά ως μεγέθη.
// Κάθε re-hash θα γίνεται βάσει αυτής της λίστας. Αν χρειάζονται παραπάνω απο 1610612741 στοχεία, τότε σε καθε rehash διπλασιάζουμε το μέγεθος.
int prime_sizes[] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241,
	786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741};

// Χρησιμοποιούμε open addressing, οπότε σύμφωνα με την θεωρία, πρέπει πάντα να διατηρούμε
// τον load factor του  hash table μικρότερο ή ίσο του 0.5, για να έχουμε αποδoτικές πράξεις
#define MAX_LOAD_FACTOR 0.5

// Κάθε θέση i θεωρείται γεινοτική με όλες τις θέσεις μέχρι και την i + NEIGHBOURS
#define NEIGHBOURS 3


// Δομή του κάθε κόμβου που έχει το hash table (με το οποίο υλοιποιούμε το map)
struct map_node{
	Pointer key;		// Το κλειδί που χρησιμοποιείται για να hash-αρουμε
	Pointer value;  	// Η τιμή που αντισοιχίζεται στο παραπάνω κλειδί
	State state;		// Μεταβλητή για να μαρκάρουμε την κατάσταση των κόμβων
	bool in_vector_flag;// true αν το node έχει τοποθετηθεί σε vector (βοηθά στο map_next)
};

// Δομή του Map (περιέχει όλες τις πληροφορίες που χρεαζόμαστε για το HashTable)
struct map {
	MapNode array;				// Ο πίνακας που θα χρησιμοποιήσουμε για το map (remember, φτιάχνουμε ένα hash table)
	Vector* vector_array;		// Ο πίνακας που θα χρησιμοποιηθεί για το vector του κάθε mapnode που ανήκει στο main array (separate-chaining)
	int capacity;				// Πόσο χώρο έχουμε δεσμεύσει.
	int size;					// Πόσα στοιχεία έχουμε προσθέσει
	CompareFunc compare;		// Συνάρτηση για σύγκριση δεικτών, που πρέπει να δίνεται απο τον χρήστη
	HashFunc hash_function;		// Συνάρτηση για να παίρνουμε το hash code του κάθε αντικειμένου.
	DestroyFunc destroy_key;	// Συναρτήσεις που καλούνται όταν διαγράφουμε έναν κόμβο απο το map.
	DestroyFunc destroy_value;
};


Map map_create(CompareFunc compare, DestroyFunc destroy_key, DestroyFunc destroy_value) {
	// Δεσμεύουμε κατάλληλα τον χώρο που χρειαζόμαστε για το hash table
	Map map = malloc(sizeof(*map));
	map->capacity = prime_sizes[0];
	map->array = malloc(map->capacity * sizeof(struct map_node));
	map->vector_array = malloc(map->capacity * sizeof(Vector));

	// Αρχικοποιούμε τους κόμβους που έχουμε σαν διαθέσιμους.
	for (int i = 0; i < map->capacity; i++)
		map->array[i].state = EMPTY;

	// Αρχικοποιούμε όλους τους pointers σε vector ως κενά vectors
	for (int i = 0; i < map->capacity; i++)
		map->vector_array[i] = vector_create(0, NULL);

	map->size = 0;
	map->compare = compare;
	map->destroy_key = destroy_key;
	map->destroy_value = destroy_value;

	return map;
}

// Επιστρέφει τον αριθμό των entries του map σε μία χρονική στιγμή.
int map_size(Map map) {
	return map->size;
}

// Συνάρτηση για την επέκταση του Hash Table σε περίπτωση που ο load factor μεγαλώσει πολύ.
static void rehash(Map map) {
	// Αποθήκευση των παλιών δεδομένων
	int old_capacity = map->capacity;
	MapNode old_array = map->array;
	Vector* old_vector_array = map->vector_array;

	// Βρίσκουμε τη νέα χωρητικότητα, διασχίζοντας τη λίστα των πρώτων ώστε να βρούμε τον επόμενο. 
	int prime_no = sizeof(prime_sizes) / sizeof(int);	// το μέγεθος του πίνακα
	for (int i = 0; i < prime_no; i++) {					// LCOV_EXCL_LINE
		if (prime_sizes[i] > old_capacity) {
			map->capacity = prime_sizes[i]; 
			break;
		}
	}
	// Αν έχουμε εξαντλήσει όλους τους πρώτους, διπλασιάζουμε
	if (map->capacity == old_capacity)					// LCOV_EXCL_LINE
		map->capacity *= 2;								// LCOV_EXCL_LINE

	// Δημιουργούμε ένα μεγαλύτερο hash table
	map->array = malloc(map->capacity * sizeof(struct map_node));
	for (int i = 0; i < map->capacity; i++)
		map->array[i].state = EMPTY;

	// Αρχικοποιούμε όλους τους pointers σε vector ως κενά vectors
	map->vector_array = malloc(map->capacity * sizeof(Vector));
	for (int i = 0; i < map->capacity; i++)
		map->vector_array[i] = vector_create(0, NULL);

	// Τοποθετούμε ΜΟΝΟ τα entries που όντως περιέχουν ένα στοιχείο
	map->size = 0;
	for (int i = 0; i < old_capacity; i++)	{
		if (old_array[i].state == OCCUPIED)
			map_insert(map, old_array[i].key, old_array[i].value);
		Vector current_vector = old_vector_array[i];
		for (VectorNode node = vector_first(current_vector);
			 node != VECTOR_EOF;
			 node = vector_next(current_vector, node)) {
			MapNode map_node = vector_node_value(current_vector, node);
			if (map_node->state == OCCUPIED)
				map_insert(map, map_node->key, map_node->value);
		}	
	}

	//Αποδεσμεύουμε τον παλιό πίνακα ώστε να μήν έχουμε leaks
	free(old_array);
	for (int i = 0; i < old_capacity; i++)
	{
		vector_destroy(old_vector_array[i]);
	}	
	free(old_vector_array);
}


// Εισαγωγή στοιχείου στο αντίστοιχο vector (separate-chaining) βοηθητική για map_insert
void insert_in_vector(Map map, Pointer key, Pointer value) {
	uint pos = map->hash_function(key) % map->capacity;
	MapNode node = malloc(sizeof(struct map_node));
	node->state = OCCUPIED;
	node->key = key;
	node->value = value;
	node->in_vector_flag = true;
	vector_insert_last(map->vector_array[pos], node);
}

// Εισαγωγή στο hash table του ζευγαριού (key, item). Αν το key υπάρχει,
// ανανέωση του με ένα νέο value
void map_insert(Map map, Pointer key, Pointer value) {
	// Ελέγχουμε κατευθείαν αν υπάρχει το key στο map
	MapNode node = map_find_node(map, key);
	if (node != MAP_EOF) {
		if (node->key != key && map->destroy_key != NULL)
			map->destroy_key(node->key);

		if (node->value != value && map->destroy_value != NULL)
			map->destroy_value(node->value);

		node->key = key;
		node->value = value;
		return;
	}

	// Σε αυτό το σημείο δεν υπάρχει ίδιο key στο map
	uint pos;
	node = NULL;

	// Αρχικά ελέγχουμε τους i+NEIGHBOURS κόμβους
	int count = 0;
	for (pos = map->hash_function(key) % map->capacity;		// ξεκινώντας από τη θέση που κάνει hash το key
		map->array[pos].state != EMPTY;						// αν φτάσουμε σε EMPTY σταματάμε
		pos = (pos + 1) % map->capacity, count++) {			// linear probing, γυρνώντας στην αρχή όταν φτάσουμε στη τέλος του πίνακα
		if (count == NEIGHBOURS) {
			insert_in_vector(map, key, value);
			pos = map->hash_function(key) % map->capacity;
			break;
		}
	}

	if (map->array[pos].state == EMPTY)						// αν βρήκαμε EMPTY, το node δεν έχει πάρει ακόμα τιμή
		node = &map->array[pos];

	map->size++;

	// Προσθήκη τιμών στον κόμβο
	if (node != NULL) {
		node->state = OCCUPIED;
		node->key = key;
		node->value = value;
		node->in_vector_flag = false;
	}

	// Αν με την νέα εισαγωγή ξεπερνάμε το μέγιστο load factor, πρέπει να κάνουμε rehash.
	// Στο load factor μετράμε και τα DELETED, γιατί και αυτά επηρρεάζουν τις αναζητήσεις.
	float load_factor = (float) map->size / map->capacity;
	if (load_factor > MAX_LOAD_FACTOR)
		rehash(map);
}

// Διαγραφή απο το Hash Table του κλειδιού με τιμή key
bool map_remove(Map map, Pointer key) {
	uint pos = map->hash_function(key) % map->capacity;
	
	// Αρχικά ελέγχουμε τους i+NEIGHBOURS κόμβους
	int count = 0;
	for (pos = map->hash_function(key) % map->capacity;	
		count != NEIGHBOURS + 1;								
		pos = (pos + 1) % map->capacity, count++) {
		MapNode node = &map->array[pos];
		if(node->state == OCCUPIED)
			if (map->compare(node->key, key) == 0) {
				node->state = EMPTY;
				map->size--;
				// destroy
				if (map->destroy_key != NULL)
					map->destroy_key(node->key);
				if (map->destroy_value != NULL)
					map->destroy_value(node->value);
				return true;
			}
	}

	// Κάνουμε reset το position και ελέγχουμε το vector
	pos = map->hash_function(key) % map->capacity;
	Vector vector = map->vector_array[pos];
	count = 0;
	for (VectorNode node = vector_first(vector);
		 node != VECTOR_EOF;
		 node = vector_next(vector, node), count++) {
		MapNode map_node = vector_node_value(vector, node);
		if (map->compare(map_node->key, key) == 0)	{
			map->size--;
			// Αφαιρούμε το συγκεκριμένο node με swap και pop_back
			MapNode last_vector_node_value = vector_node_value(vector, vector_last(vector));
			vector_set_at(vector, count, last_vector_node_value);
			vector_remove_last(vector);
			return true;
		}
	}
	return false;
}

// Αναζήτηση στο map, με σκοπό να επιστραφεί το value του κλειδιού που περνάμε σαν όρισμα.

Pointer map_find(Map map, Pointer key) {
	MapNode node = map_find_node(map, key);
	if (node != MAP_EOF)
		return node->value;
	else
		return NULL;
}


DestroyFunc map_set_destroy_key(Map map, DestroyFunc destroy_key) {
	DestroyFunc old = map->destroy_key;
	map->destroy_key = destroy_key;
	return old;
}

DestroyFunc map_set_destroy_value(Map map, DestroyFunc destroy_value) {
	DestroyFunc old = map->destroy_value;
	map->destroy_value = destroy_value;
	return old;
}

// Απελευθέρωση μνήμης που δεσμεύει το map
void map_destroy(Map map) {
	for (int i = 0; i < map->capacity; i++) {
		if (map->array[i].state == OCCUPIED) {
			if (map->destroy_key != NULL)
				map->destroy_key(map->array[i].key);
			if (map->destroy_value != NULL)
				map->destroy_value(map->array[i].value);
		}
		vector_destroy(map->vector_array[i]);
	}
	free(map->vector_array);
	free(map->array);
	free(map);
}

/////////////////////// Διάσχιση του map μέσω κόμβων ///////////////////////////

MapNode map_first(Map map) {
    for (int i = 0; i < map->capacity; i++)
        if (map->array[i].state == OCCUPIED)
            return &map->array[i];
    return MAP_EOF;
}

MapNode map_next(Map map, MapNode node) {
	bool position_reset = false; // Αν true ψάχνουμε τα vectors απτήν αρχή

	if (node->in_vector_flag == false) {
        // Το node είναι pointer στο i-οστό στοιχείο του array, οπότε node - array == i  (pointer arithmetic!)
        for (int i = node - map->array + 1; i < map->capacity; i++)
            if (map->array[i].state == OCCUPIED)
                return &map->array[i];
        position_reset = true;    // Αφού δεν είναι στο map_array θα ψαξω στα vectors απτήν αρχή
    }

    bool next_node = position_reset ? true : false;		// Αν true ψάχνω πρώτα το κανονικ΄΄ο node και μετά θα βρω το next
	uint pos = map->hash_function(node->key) % map->capacity;
    for (int position = position_reset ? 0 : pos;
        position < map->capacity;
        position++) {
        if (vector_size(map->vector_array[position]) != 0)
            for (VectorNode vector_node = vector_first(map->vector_array[position]);    
                vector_node != VECTOR_EOF;
                vector_node = vector_next(map->vector_array[position], vector_node)) {
				MapNode map_node = vector_node_value(map->vector_array[position], vector_node);
				if (next_node && map_node->state == OCCUPIED)
					return map_node;
				if (map_node->state == OCCUPIED)
					if (map->compare(map_node->key, node->key) == 0)
						next_node = true;
			}
	}
	return MAP_EOF;
}

Pointer map_node_key(Map map, MapNode node) {
	return node->key;
}

Pointer map_node_value(Map map, MapNode node) {
	return node->value;
}

MapNode map_find_node(Map map, Pointer key) {
	uint pos = map->hash_function(key) % map->capacity;
	
	// Αρχικά ελέγχουμε τους i+NEIGHBOURS κόμβους
	int count = 0;
	for (pos = map->hash_function(key) % map->capacity;	
		count != NEIGHBOURS + 1;								
		pos = (pos + 1) % map->capacity, count++) {
		if (map->array[pos].state == OCCUPIED) {
			if (map->compare(map->array[pos].key, key) == 0)
				return &map->array[pos];
		}
	}

	// Κάνουμε reset το position και ελέγχουμε το vector
	pos = map->hash_function(key) % map->capacity;
	Vector vector = map->vector_array[pos];
	for (VectorNode node = vector_first(vector);
		 node != VECTOR_EOF;
		 node = vector_next(vector, node)) {
		MapNode map_node = vector_node_value(vector, node);
		if (map_node->state == OCCUPIED)
			if (map->compare(map_node->key, key) == 0)
				return map_node;
	}
	return MAP_EOF;
}

// Αρχικοποίηση της συνάρτησης κατακερματισμού του συγκεκριμένου map.
void map_set_hash_function(Map map, HashFunc func) {
	map->hash_function = func;
}

uint hash_string(Pointer value) {
	// djb2 hash function, απλή, γρήγορη, και σε γενικές γραμμές αποδοτική
    uint hash = 5381;
    for (char* s = value; *s != '\0'; s++)
		hash = (hash << 5) + hash + *s;			// hash = (hash * 33) + *s. Το foo << 5 είναι γρηγορότερη εκδοχή του foo * 32.
    return hash;
}

uint hash_int(Pointer value) {
	return *(int*)value;
}

uint hash_pointer(Pointer value) {
	return (size_t)value;				// cast σε sizt_t, που έχει το ίδιο μήκος με έναν pointer
}
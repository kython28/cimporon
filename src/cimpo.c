#include "cimpo.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>

typedef struct {
	uint64_t key;
	int64_t value;
	uint64_t ptr_s;
	uint64_t ptr_b;
} cimpo_node;

cimpo *openFile(const char *name){
	if (name == NULL){
		return NULL;
	}
	cimpo *file = (cimpo*) calloc(1, sizeof(cimpo));
	uint64_t sig, size, CIMPO_SIGNATURE;
	CIMPO_SIGNATURE = 0x0000004F504D4943;

	file->fd = open(name, O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (file->fd < 0){
		file->fd = open(name, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (file->fd < 0){
			free(file);
			return NULL;	
		}
	}

	size = lseek(file->fd, 0, SEEK_END);
	if (size == 0){
		size = write(file->fd, &CIMPO_SIGNATURE, 8);
		if (size < 8){
			closeCimpoFile(file);
			return NULL;
		}
	}else{
		lseek(file->fd, 0, SEEK_SET);
		size = read(file->fd, &sig, 8);
		if (size < 8){
			closeCimpoFile(file);
			return NULL;
		}else if (sig != CIMPO_SIGNATURE){
			closeCimpoFile(file);
			return NULL;
		}
	}

	lseek(file->fd, 0, SEEK_SET);
	file->size = lseek(file->fd, 0, SEEK_END);
	lseek(file->fd, 8, SEEK_SET);
	return file;
}

void closeCimpoFile(cimpo *file){
	if (file == NULL){
		return;
	}
	close(file->fd);
	free(file);
}

uint8_t writeNode(cimpo *file, cimpo_node *node){
	uint64_t s = write(file->fd, node, 32);
	if (s < 32){
		return 1;
	}
	return 0;
}

uint8_t readNode(cimpo *file, cimpo_node *node){
	uint64_t curr = lseek(file->fd, 0, SEEK_CUR);
	uint64_t s = read(file->fd, node, 32);
	if (s < 32){
		return 1;
	}
	lseek(file->fd, curr, SEEK_SET);
	return 0;
}

int64_t getValue(cimpo *file, uint64_t key){
	if (file == NULL || key == 0){
		return 1;
	}
	uint64_t curr;

	cimpo_node node;
	memset(&node, 0, 32);

	if (file->size > 8){
		while (1){
			curr = lseek(file->fd, 0, SEEK_CUR);
			if (readNode(file, &node)){
				lseek(file->fd, 8, SEEK_SET);
				return 1;
			}
			if (node.key == 0){
				lseek(file->fd, 8, SEEK_SET);
				return 0;
			}else if (key > node.key && node.ptr_b == 0){
				lseek(file->fd, 8, SEEK_SET);
				return 0;
			}else if (key < node.key && node.ptr_s == 0){
				lseek(file->fd, 8, SEEK_SET);
				return 0;
			}

			if (key > node.key){
				lseek(file->fd, node.ptr_b, SEEK_SET);
			}else if (key < node.key){
				lseek(file->fd, node.ptr_s, SEEK_SET);
			}else{
				break;
			}
		}	
	}
	lseek(file->fd, 8, SEEK_SET);
	return node.value;
}

uint8_t addValue(cimpo *file, uint64_t key, int64_t value){
	if (file == NULL || key == 0){
		return 1;
	}
	uint64_t curr;

	cimpo_node node;
	memset(&node, 0, 32);

	if (file->size > 8){
		while (1){
			curr = lseek(file->fd, 0, SEEK_CUR);
			if (readNode(file, &node)){
				lseek(file->fd, 8, SEEK_SET);
				return 1;
			}
			if (node.key == 0){
				break;
			}else if (key > node.key && node.ptr_b == 0){
				lseek(file->fd, 0, SEEK_SET);
				node.ptr_b = lseek(file->fd, 0, SEEK_END);
				lseek(file->fd, curr, SEEK_SET);
				writeNode(file, &node);
				node.ptr_b = 0;
				node.ptr_s = 0;
				lseek(file->fd, 0, SEEK_END);
				break;
			}else if (key < node.key && node.ptr_s == 0){
				lseek(file->fd, 0, SEEK_SET);
				node.ptr_s = lseek(file->fd, 0, SEEK_END);
				lseek(file->fd, curr, SEEK_SET);
				writeNode(file, &node);
				node.ptr_b = 0;
				node.ptr_s = 0;
				lseek(file->fd, 0, SEEK_END);
				break;
			}

			if (key > node.key){
				lseek(file->fd, node.ptr_b, SEEK_SET);
			}else if (key < node.key){
				lseek(file->fd, node.ptr_s, SEEK_SET);
			}else{
				lseek(file->fd, 8, SEEK_SET);
				return 1;
			}
		}	
	}
	node.key = key;
	node.value = value;
	if (writeNode(file, &node)){
		lseek(file->fd, 8, SEEK_SET);
		return 1;
	}
	lseek(file->fd, 0, SEEK_SET);
	file->size = lseek(file->fd, 0, SEEK_END);
	lseek(file->fd, 8, SEEK_SET);
	return 0;
}

uint8_t editValue(cimpo *file, uint64_t key, int64_t value){
	if (file == NULL || key == 0){
		return 1;
	}
	uint64_t curr;

	cimpo_node node;
	memset(&node, 0, 32);

	if (file->size > 8){
		while (1){
			curr = lseek(file->fd, 0, SEEK_CUR);
			if (readNode(file, &node)){
				lseek(file->fd, 8, SEEK_SET);
				return 1;
			}
			if (node.key == 0){
				lseek(file->fd, 8, SEEK_SET);
				return 1;
			}else if (key > node.key && node.ptr_b == 0){
				lseek(file->fd, 8, SEEK_SET);
				return 1;
			}else if (key < node.key && node.ptr_s == 0){
				lseek(file->fd, 8, SEEK_SET);
				return 1;
			}

			if (key > node.key){
				lseek(file->fd, node.ptr_b, SEEK_SET);
			}else if (key < node.key){
				lseek(file->fd, node.ptr_s, SEEK_SET);
			}else{
				break;
			}
		}	
	}else{
		lseek(file->fd, 8, SEEK_SET);
		return 1;
	}

	node.value = value;
	if (writeNode(file, &node)){
		lseek(file->fd, 8, SEEK_SET);
		return 1;
	}
	lseek(file->fd, 8, SEEK_SET);
	return 0;
}

uint8_t removeKey(cimpo *file, uint64_t key){
	if (file == NULL || key == 0){
		return 1;
	}
	uint64_t curr, curr2=0, curr3;

	cimpo_node node, node2, node3;
	memset(&node, 0, 32);

	if (file->size > 8){
		while (1){
			curr = lseek(file->fd, 0, SEEK_CUR);
			if (readNode(file, &node)){
				lseek(file->fd, 8, SEEK_SET);
				return 1;
			}
			if (node.key == 0){
				lseek(file->fd, 8, SEEK_SET);
				return 1;
			}else if (key > node.key && node.ptr_b == 0){
				lseek(file->fd, 8, SEEK_SET);
				return 1;
			}else if (key < node.key && node.ptr_s == 0){
				lseek(file->fd, 8, SEEK_SET);
				return 1;
			}

			if (key > node.key){
				lseek(file->fd, node.ptr_b, SEEK_SET);
			}else if (key < node.key){
				lseek(file->fd, node.ptr_s, SEEK_SET);
			}else{
				curr3 = curr;
				memcpy(&node2, &node, 32);
				break;
			}
		}
		if (node2.ptr_b != 0){
			lseek(file->fd, node2.ptr_b, SEEK_SET);
			while (1){
				curr = lseek(file->fd, 0, SEEK_CUR);
				if (readNode(file, &node)){
					lseek(file->fd, 8, SEEK_SET);
					return 1;
				}
				if (node.key == 0){
					lseek(file->fd, curr2, SEEK_SET);
					break;
				}else if (node.ptr_s == 0){
					curr2 = curr;
					lseek(file->fd, curr2, SEEK_SET);
					if (readNode(file, &node)){
						lseek(file->fd, 8, SEEK_SET);
						return 1;
					}
					break;
				}

				curr2 = curr;
				lseek(file->fd, node.ptr_s, SEEK_SET);
			}
		}else if (node2.ptr_s != 0){
			lseek(file->fd, node2.ptr_s, SEEK_SET);
			while (1){
				curr = lseek(file->fd, 0, SEEK_CUR);
				if (readNode(file, &node)){
					lseek(file->fd, 8, SEEK_SET);
					return 1;
				}
				if (node.key == 0){
					lseek(file->fd, curr2, SEEK_SET);
					break;
				}else if (node.ptr_b == 0){
					curr2 = curr;
					lseek(file->fd, curr2, SEEK_SET);
					if (readNode(file, &node)){
						lseek(file->fd, 8, SEEK_SET);
						return 1;
					}
					break;
				}

				curr2 = curr;
				lseek(file->fd, node.ptr_b, SEEK_SET);
			}
		}else{
			curr2 = 0;
		}
	}

	if (curr2 > 0){
		node2.key = node.key;
		node2.value = node.value;
		
		if (curr2 != curr3){
			lseek(file->fd, 8, SEEK_SET);
			if (removeKey(file, node.key)){
				return 1;
			}
		}
		lseek(file->fd, curr3, SEEK_SET);
		if (writeNode(file, &node2)){
			lseek(file->fd, 8, SEEK_SET);
			return 1;
		}
	}else{
		lseek(file->fd, curr3, SEEK_SET);
		node2.key = 0;
		node2.value = 0;
		if (writeNode(file, &node2)){
			lseek(file->fd, 8, SEEK_SET);
			return 1;
		}
	}
	lseek(file->fd, 8, SEEK_SET);
	return 0;
}
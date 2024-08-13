#include "BLI_stack.h"
#include <cstddef>
#include <cstdlib> // for malloc, free
#include <cstring> // for memcpy
#include <cassert> // for assert

#define CHUNK_ELEM_MIN 32
#define CHUNK_EMPTY ((size_t)-1)
#define CHUNK_SIZE_DEFAULT (1 << 16)

class Stack {
private:
    struct StackChunk {
        StackChunk *next;
        char data[1];
    };

    StackChunk *chunk_curr;
    StackChunk *chunk_free;
    size_t chunk_index;
    size_t chunk_elem_max;
    size_t elem_size;
    size_t elem_num;

    void *get_last_elem() {
        if (chunk_curr == nullptr || chunk_index == CHUNK_EMPTY) {
            return nullptr;
        }
        return chunk_curr->data + (elem_size * chunk_index);
    }

    void free_chunks(StackChunk *chunk) {
        while (chunk) {
            StackChunk *next_chunk = chunk->next;
            free(chunk);
            chunk = next_chunk;
        }
    }

    size_t calculate_chunk_elem_max(size_t elem_size, size_t chunk_size) {
        const size_t elem_size_min = elem_size * CHUNK_ELEM_MIN;
        assert((elem_size != 0) && (chunk_size != 0));

        while (chunk_size <= elem_size_min) {
            chunk_size <<= 1;
        }

        chunk_size -= (sizeof(StackChunk));
        return chunk_size / elem_size;
    }

public:
    Stack(size_t elem_size, const char *description, size_t chunk_size)
        : chunk_curr(nullptr), chunk_free(nullptr), chunk_index(CHUNK_EMPTY), 
          elem_size(elem_size), elem_num(0) {
        chunk_elem_max = calculate_chunk_elem_max(elem_size, chunk_size);
    }

    ~Stack() {
        free_chunks(chunk_curr);
        free_chunks(chunk_free);
    }

    void *push_raw() {
        if (chunk_index == CHUNK_EMPTY) {
            StackChunk *new_chunk;
            if (chunk_free) {
                new_chunk = chunk_free;
                chunk_free = new_chunk->next;
            } else {
                new_chunk = static_cast<StackChunk*>(malloc(sizeof(StackChunk) + (elem_size * chunk_elem_max)));
            }
            new_chunk->next = chunk_curr;
            chunk_curr = new_chunk;
            chunk_index = 0;
        } else {
            chunk_index++;
            if (chunk_index == chunk_elem_max) {
                // Allocate new chunk and reset index
                StackChunk *new_chunk;
                if (chunk_free) {
                    new_chunk = chunk_free;
                    chunk_free = new_chunk->next;
                } else {
                    new_chunk = static_cast<StackChunk*>(malloc(sizeof(StackChunk) + (elem_size * chunk_elem_max)));
                }
                new_chunk->next = chunk_curr;
                chunk_curr = new_chunk;
                chunk_index = 0;
            }
        }

        assert(chunk_curr != nullptr && chunk_index < chunk_elem_max);

        elem_num++;
        return get_last_elem();
    }

    void push(const void *src) {
        void *dst = push_raw();
        if (dst) { 
            memcpy(dst, src, elem_size);
        }
    }

    void pop(void *dst) {
        if (is_empty()) {
            memset(dst, 0, elem_size);
            return;
        }
        void *src = get_last_elem();
        if (src) {
            memcpy(dst, src, elem_size);
            discard();
        }
    }

    void pop_n(void *dst, unsigned int n) {
        assert(n <= count());
        while (n--) {
            pop(dst);
            dst = static_cast<void *>(static_cast<char *>(dst) + elem_size);
        }
    }

    void pop_n_reverse(void *dst, unsigned int n) {
        assert(n <= count());
        dst = static_cast<void *>(static_cast<char *>(dst) + (elem_size * n));
        while (n--) {
            dst = static_cast<void *>(static_cast<char *>(dst) - elem_size);
            pop(dst);
        }
    }

    void *peek() {
        if (is_empty()) {
            return nullptr;
        }
        return get_last_elem();
    }

    void discard() {
        if (is_empty()) {
            return;
        }
        elem_num--;
        if (--chunk_index == CHUNK_EMPTY) {
            StackChunk *to_free = chunk_curr;
            chunk_curr = chunk_curr->next;
            to_free->next = chunk_free;
            chunk_free = to_free;
            chunk_index = chunk_elem_max - 1;
        }
    }

    void clear() {
        elem_num = 0;
        chunk_index = CHUNK_EMPTY;

        if (chunk_free) {
            if (chunk_curr) {
                StackChunk *last_free = chunk_free;
                while (last_free->next) {
                    last_free = last_free->next;
                }
                last_free->next = chunk_curr;
                chunk_curr = nullptr;
            }
        } else {
            chunk_free = chunk_curr;
            chunk_curr = nullptr;
        }
    }

    size_t count() const {
        return elem_num;
    }

    bool is_empty() const {
        return chunk_curr == nullptr;
    }
};

extern "C" {

BLI_Stack *BLI_stack_new_ex(size_t elem_size, const char *description, size_t chunk_size) {
    return reinterpret_cast<BLI_Stack *>(new Stack(elem_size, description, chunk_size));
}

BLI_Stack *BLI_stack_new(size_t elem_size, const char *description) {
    return BLI_stack_new_ex(elem_size, description, CHUNK_SIZE_DEFAULT);
}

void BLI_stack_free(BLI_Stack *stack) {
    delete reinterpret_cast<Stack *>(stack);
}

void *BLI_stack_push_r(BLI_Stack *stack) {
    return reinterpret_cast<Stack *>(stack)->push_raw();
}

void BLI_stack_push(BLI_Stack *stack, const void *src) {
    reinterpret_cast<Stack *>(stack)->push(src);
}

void BLI_stack_pop(BLI_Stack *stack, void *dst) {
    reinterpret_cast<Stack *>(stack)->pop(dst);
}

void BLI_stack_pop_n(BLI_Stack *stack, void *dst, unsigned int n) {
    reinterpret_cast<Stack *>(stack)->pop_n(dst, n);
}

void BLI_stack_pop_n_reverse(BLI_Stack *stack, void *dst, unsigned int n) {
    reinterpret_cast<Stack *>(stack)->pop_n_reverse(dst, n);
}

void *BLI_stack_peek(BLI_Stack *stack) {
    return reinterpret_cast<Stack *>(stack)->peek();
}

void BLI_stack_discard(BLI_Stack *stack) {
    reinterpret_cast<Stack *>(stack)->discard();
}

void BLI_stack_clear(BLI_Stack *stack) {
    reinterpret_cast<Stack *>(stack)->clear();
}

size_t BLI_stack_count(const BLI_Stack *stack) {
    return reinterpret_cast<const Stack *>(stack)->count();
}

bool BLI_stack_is_empty(const BLI_Stack *stack) {
    return reinterpret_cast<const Stack *>(stack)->is_empty();
}

}

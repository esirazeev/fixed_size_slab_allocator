#ifndef FIXED_SIZE_SLAB_ALLOCATOR_HPP
#define FIXED_SIZE_SLAB_ALLOCATOR_HPP

#include <cstdint>
#include <utility>

template <typename T, uint32_t N>
class fixed_size_slab_allocator final
{
public:
	fixed_size_slab_allocator() noexcept : size_{N}
	{
		head_ = reinterpret_cast<node*>(mem_buffer_);
		auto cur_node{head_};

		for (int32_t i{0}; i < (node_size_ * N) - node_size_; i += node_size_)
		{
			cur_node->next = reinterpret_cast<node*>(&mem_buffer_[i + node_size_]);
			cur_node = cur_node->next;
		}

		cur_node->next = nullptr;
	}

	template <typename... Args>
	T* allocate_and_create(Args&&... args)
	{
		if (size_)
		{
			auto next{head_->next};
			auto obj{new (head_) T(std::forward<Args>(args)...)};

			head_ = next;
			--size_;

			return obj;
		}

		return {};
	}

	void deallocate(T*& obj)
	{
		if (obj && reinterpret_cast<uint8_t*>(obj) >= mem_buffer_ && reinterpret_cast<uint8_t*>(obj) < mem_buffer_ + (node_size_ * N))
		{
			++size_;
			obj->~T();

			const auto free_block_ptr{reinterpret_cast<node*>(obj)};
			free_block_ptr->next = head_;
			head_ = free_block_ptr;

			obj = nullptr;
		}
	}

	uint32_t size() const noexcept
	{
		return size_;
	}

private:
	struct node
	{
		node* next;
	};

	static constexpr auto node_size_{(sizeof(T) > sizeof(void*) ? sizeof(T) : sizeof(void*))};

	node* head_;
	uint8_t mem_buffer_[node_size_ * N]{};
	uint32_t size_;
};

#endif // FIXED_SIZE_SLAB_ALLOCATOR_HPP

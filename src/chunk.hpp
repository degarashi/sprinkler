#pragma once
#include <vector>
#include <list>
#include <algorithm>
#include <stdexcept>
#include <cassert>

namespace dg {
	template <class Val, class Key>
	class Chunk {
		private:
			using value_t = Val;
			using ValueV = std::vector<value_t>;
			struct Bucket {
				Key		key;
				ValueV	value;
			};
			using BucketL = std::list<Bucket>;
			using Itr = typename BucketL::iterator;
			struct Cursor {
				Itr		bucket;
				size_t	index;
			};

			BucketL		_array;
			Cursor		_cursor;
			size_t		_total;

			void _skipEmpty() {
				assert(_cursor.index == 0);
				const auto itrE = _array.end();
				auto& itr = _cursor.bucket;
				while(itr != itrE &&
						itr->value.empty())
					++itr;
			}

		public:
			Chunk():
				_cursor{
					.bucket = _array.end(),
					.index = 0
				},
				_total(0)
			{}
			// リストの末尾にBucketを追加
			template <class V>
			void addBucket(const Key& key, V&& v) {
				_total += v.size();
				_array.emplace_back(Bucket{key, std::forward<V>(v)});
				if(_array.size() == 1) {
					_cursor.bucket = _array.begin();
					_skipEmpty();
				} else if(isEnd()) {
					assert(_cursor.index == 0);
					_cursor.bucket = _array.end();
					--_cursor.bucket;
				}
			}
			// 任意のキーを持つBucketを削除
			void remBucket(const Key& key) {
				auto itr = std::find_if(
					_array.begin(),
					_array.end(),
					[&key](const auto& b){
						return key == b.key;
					}
				);
				if(itr == _array.end())
					throw std::runtime_error("no such key");

				_total -= itr->value.size();
				// 現在選択中のBucketだったらその直後へカーソルをずらす
				const bool referenced = _cursor.bucket == itr;
				_cursor.bucket = _array.erase(itr);
				if(referenced) {
					_cursor.index = 0;
					_skipEmpty();
				}
			}
			void resetCursor() noexcept {
				_cursor.bucket = _array.begin();
				_cursor.index = 0;
			}
			bool isEnd() const noexcept {
				return _cursor.bucket == _array.end();
			}
			void previous() {
				if(_array.empty())
					return;

				if(_cursor.index == 0) {
					if(_cursor.bucket
				} else
					--_cursor.index;
			}
			const value_t& advance() {
				if(isEnd())
					throw std::out_of_range("cursor is at end");
				auto& itr = _cursor.bucket;
				const value_t& ret = itr->value.at(_cursor.index);
				if(++_cursor.index == itr->value.size()) {
					++itr;
					_cursor.index = 0;
					_skipEmpty();
				}
				return ret;
			}
			size_t cursor() const noexcept {
				if(isEnd()) {
					assert(_cursor.index == 0);
					return _total;
				}

				size_t ret = 0;
				auto itr = _array.begin();
				while(itr != _array.end() &&
					itr != _cursor.bucket)
				{
					ret += itr->value.size();
					++itr;
				}
				return ret + _cursor.index;
			}
			size_t total() const noexcept {
				return _total;
			}
	};
}

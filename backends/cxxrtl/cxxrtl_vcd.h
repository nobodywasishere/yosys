/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2020  whitequark <whitequark@whitequark.org>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#ifndef CXXRTL_VCD_H
#define CXXRTL_VCD_H

#include <backends/cxxrtl/cxxrtl.h>

namespace cxxrtl {

class vcd_writer {
	struct variable {
		size_t ident;
		size_t width;
		chunk_t *curr;
		size_t prev_off;
	};

	std::vector<std::string> current_scope;
	std::vector<variable> variables;
	std::vector<chunk_t> cache;
	bool streaming = false;

	void emit_timescale(unsigned number, const std::string &unit) {
		assert(!streaming);
		assert(number == 1 || number == 10 || number == 100);
		assert(unit == "s" || unit == "ms" || unit == "us" ||
		       unit == "ns" || unit == "ps" || unit == "fs");
		buffer += "$timescale " + std::to_string(number) + " " + unit + " $end\n";
	}

	void emit_scope(const std::vector<std::string> &scope) {
		assert(!streaming);
		while (current_scope.size() > scope.size() ||
		       (current_scope.size() > 0 &&
			current_scope[current_scope.size() - 1] != scope[current_scope.size() - 1])) {
			buffer += "$upscope $end\n";
			current_scope.pop_back();
		}
		while (current_scope.size() < scope.size()) {
			buffer += "$scope module " + scope[current_scope.size()] + " $end\n";
			current_scope.push_back(scope[current_scope.size()]);
		}
	}

	void emit_ident(size_t ident) {
		do {
			buffer += '!' + ident % 94; // "base94"
			ident /= 94;
		} while (ident != 0);
	}

	void emit_var(const variable &var, const std::string &type, const std::string &name) {
		assert(!streaming);
		buffer += "$var " + type + " " + std::to_string(var.width) + " ";
		emit_ident(var.ident);
		buffer += " " + name + " $end\n";
	}

	void emit_enddefinitions() {
		assert(!streaming);
		buffer += "$enddefinitions $end\n";
		streaming = true;
	}

	void emit_time(uint64_t timestamp) {
		assert(streaming);
		buffer += "#" + std::to_string(timestamp) + "\n";
	}

	void emit_scalar(const variable &var) {
		assert(streaming);
		assert(var.width == 1);
		buffer += (*var.curr ? '1' : '0');
		emit_ident(var.ident);
		buffer += '\n';
	}

	void emit_vector(const variable &var) {
		assert(streaming);
		buffer += 'b';
		for (size_t bit = var.width - 1; bit != (size_t)-1; bit--) {
			bool bit_curr = var.curr[bit / (8 * sizeof(chunk_t))] & (1 << (bit % (8 * sizeof(chunk_t))));
			buffer += (bit_curr ? '1' : '0');
		}
		buffer += ' ';
		emit_ident(var.ident);
		buffer += '\n';
	}

	void append_variable(size_t width, chunk_t *curr) {
		const size_t chunks = (width + (sizeof(chunk_t) * 8 - 1)) / (sizeof(chunk_t) * 8);
		variables.emplace_back(variable { variables.size(), width, curr, cache.size() });
		cache.insert(cache.end(), &curr[0], &curr[chunks]);
	}

	bool test_variable(const variable &var) {
		const size_t chunks = (var.width + (sizeof(chunk_t) * 8 - 1)) / (sizeof(chunk_t) * 8);
		if (std::equal(&var.curr[0], &var.curr[chunks], &cache[var.prev_off])) {
			return false;
		} else {
			std::copy(&var.curr[0], &var.curr[chunks], &cache[var.prev_off]);
			return true;
		}
	}

	static std::vector<std::string> split_hierarchy(const std::string &hier_name) {
		std::vector<std::string> hierarchy;
		size_t prev = 0;
		while (true) {
			size_t curr = hier_name.find_first_of(' ', prev + 1);
			if (curr > hier_name.size())
				curr = hier_name.size();
			if (curr > prev + 1)
				hierarchy.push_back(hier_name.substr(prev, curr - prev));
			if (curr == hier_name.size())
				break;
			prev = curr + 1;
		}
		return hierarchy;
	}

public:
	std::string buffer;

	void timescale(unsigned number, const std::string &unit) {
		emit_timescale(number, unit);
	}

	void add(const std::string &hier_name, const debug_item &item) {
		std::vector<std::string> scope = split_hierarchy(hier_name);
		std::string name = scope.back();
		scope.pop_back();

		emit_scope(scope);
		switch (item.type) {
			// Not the best naming but oh well...
			case debug_item::VALUE:
				append_variable(item.width, item.curr);
				emit_var(variables.back(), "wire", name);
				break;
			case debug_item::WIRE:
				append_variable(item.width, item.curr);
				emit_var(variables.back(), "reg", name);
				break;
			case debug_item::MEMORY: {
				const size_t stride = (item.width + (sizeof(chunk_t) * 8 - 1)) / (sizeof(chunk_t) * 8);
				for (size_t index = 0; index < item.depth; index++) {
					chunk_t *nth_curr = &item.curr[stride * index];
					std::string nth_name = name + '[' + std::to_string(index) + ']';
					append_variable(item.width, nth_curr);
					emit_var(variables.back(), "reg", nth_name);
				}
				break;
			}
		}
	}

	template<class Filter>
	void add(const debug_items &items, const Filter &filter) {
		// `debug_items` is a map, so the items are already sorted in an order optimal for emitting
		// VCD scope sections.
		for (auto &it : items)
			if (filter(it.first, it.second))
				add(it.first, it.second);
	}

	void add(const debug_items &items) {
		this->template add(items, [](const std::string &, const debug_item &) {
			return true;
		});
	}

	void add_without_memories(const debug_items &items) {
		this->template add(items, [](const std::string &, const debug_item &item) {
			return item.type == debug_item::VALUE || item.type == debug_item::WIRE;
		});
	}

	void sample(uint64_t timestamp) {
		bool first_sample = !streaming;
		if (first_sample) {
			emit_scope({});
			emit_enddefinitions();
		}
		emit_time(timestamp);
		for (auto var : variables)
			if (test_variable(var) || first_sample) {
				if (var.width == 1)
					emit_scalar(var);
				else
					emit_vector(var);
			}
	}
};

}

#endif

#ifndef FEMTOYAML_HPP
#define FEMTOYAML_HPP

#include <algorithm>
#include <cassert>
#include <deque>
#include <memory>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <variant>
#include <vector>

namespace femtoyaml {

// thanks to https://ja.cppreference.com/w/cpp/utility/variant/visit
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

template <class Variant, class... Visitor>
auto match_with(Variant&& var, Visitor&&... visitors)
{
    return std::visit(overloaded{visitors...}, var);
}

class value;
using list = std::vector<value>;
using map = std::unordered_map<std::string, value>;

class value {
public:
    using type = std::variant<int, std::string, std::shared_ptr<list>,
                              std::shared_ptr<map>>;
    type val_;

public:
    value()
    {
    }

    value(int val) : val_(val)
    {
    }

    value(const std::string& val) : val_(val)
    {
    }

    value(const list& val) : val_(std::make_shared<list>(val))
    {
    }

    value(const map& val) : val_(std::make_shared<map>(val))
    {
    }

    template <class T>
    T& get()
    {
        return std::get<T>(val_);
    }

    template <class T>
    const T& get() const
    {
        return std::get<T>(val_);
    }

    value& operator[](int index);
    const value& operator[](int index) const;
    value& operator[](const std::string& key);
    const value& operator[](const std::string& key) const;

    template <class... Visitors>
    auto visit(Visitors&&... visitors) const
    {
        auto callback = overloaded{visitors...};
        return match_with(
            val_,
            [&](const std::shared_ptr<list>& ptr) { return callback(*ptr); },
            [&](const std::shared_ptr<map>& ptr) { return callback(*ptr); },
            [&](int val) { return callback(val); },
            [&](const std::string& str) { return callback(str); });
    }

    bool is_int() const
    {
        return std::holds_alternative<int>(val_);
    }

    bool is_string() const
    {
        return std::holds_alternative<std::string>(val_);
    }

    bool is_list() const
    {
        return std::holds_alternative<std::shared_ptr<list>>(val_);
    }

    bool is_map() const
    {
        return std::holds_alternative<std::shared_ptr<map>>(val_);
    }

    std::string to_string() const;

    std::string to_debug_string() const
    {
        std::stringstream ss;
        visit([&](int val) { ss << "int(" << val << ")"; },
              [&](const std::string& str) { ss << "string(" << str << ")"; },
              [&](const list& lst) {
                  ss << "list(";
                  for (int i = 0; i < lst.size() - 1; i++)
                      ss << lst[i].to_debug_string() << ", ";
                  ss << lst.back().to_debug_string();
                  ss << ")";
              },
              [&](const map& m) {
                  std::vector<std::string> strs;
                  for (auto&& p : m)
                      strs.emplace_back(p.first + ": " +
                                        p.second.to_debug_string());
                  ss << "map(";
                  for (int i = 0; i < strs.size() - 1; i++)
                      ss << strs[i] << ", ";
                  ss << strs.back() << ")";
              });
        return ss.str();
    }
};

template <>
list& value::get()
{
    return *std::get<std::shared_ptr<list>>(val_);
}

template <>
const list& value::get() const
{
    return *std::get<std::shared_ptr<list>>(val_);
}

template <>
map& value::get()
{
    return *std::get<std::shared_ptr<map>>(val_);
}

template <>
const map& value::get() const
{
    return *std::get<std::shared_ptr<map>>(val_);
}

value& value::operator[](int index)
{
    return get<list>().at(index);
}

const value& value::operator[](int index) const
{
    return get<list>().at(index);
}

value& value::operator[](const std::string& key)
{
    return get<map>().at(key);
}

const value& value::operator[](const std::string& key) const
{
    return get<map>().at(key);
}

namespace detail {

class stream {
private:
    std::istream& is_;
    std::deque<int> pending_;
    int column_;

private:
    int next_detail()
    {
        if (pending_.empty()) return is_.get();
        int ret = pending_.front();
        pending_.pop_front();
        return ret;
    }

public:
    stream(std::istream& is) : is_(is), column_(0)
    {
    }

    int column()
    {
        return column_;
    }

    bool eof()
    {
        return is_.eof() || is_.peek() == EOF;
    }

    int next()
    {
        int ch = next_detail();
        if (ch == '\n')
            column_ = 0;
        else
            column_++;
        return ch;
    }

    int peek(int idx = 0)
    {
        while (pending_.size() <= idx) pending_.push_back(is_.get());
        return pending_[idx];
    }

    void skip_ws()
    {
        while (true) {
            switch (peek()) {
            case ' ':
            case '\n':
                next();
                break;

            default:
                return;
            }
        }
    }
};

struct tk_eof {
};

struct tk_scalar_plain {
    std::string str;
};

struct tk_block_sequence_start {
};

struct tk_block_end {
};

struct tk_block_entry {
};

struct tk_block_mapping_start {
};

struct tk_key {
};

struct tk_value {
};

using token =
    std::variant<tk_eof, tk_scalar_plain, tk_block_sequence_start, tk_block_end,
                 tk_block_entry, tk_block_mapping_start, tk_key, tk_value>;

std::string to_string(const token& tk)
{
    return match_with(
        tk, [](const tk_eof&) { return std::string("EOF"); },
        [](const tk_scalar_plain& tk) {
            return "SCALAR(" + tk.str + ", plain)";
        },
        [](const tk_block_sequence_start&) {
            return std::string("BLOCK-SEQUENCE-START");
        },
        [](const tk_block_end&) { return std::string("BLOCK-END"); },
        [](const tk_block_entry&) { return std::string("BLOCK-ENTRY"); },
        [](const tk_block_mapping_start&) {
            return std::string("BLOCK-MAPPING-START");
        },
        [](const tk_key&) { return std::string("KEY"); },
        [](const tk_value&) { return std::string("VALUE"); });
}

class tokenizer {
private:
    stream& st_;
    std::vector<token> tks_;
    int indent_, simple_key_column_;
    std::vector<int> indents_;

private:
    bool is_blankz(int ch)
    {
        switch (ch) {
        case ' ':
        case '\n':
            return true;
        }
        return false;
    }

    tk_scalar_plain fetch_plain_scalar()
    {
        std::stringstream ss;
        while (true) {
            switch (st_.peek()) {
            case '\n':
                goto end;
            case ':':
                if (is_blankz(st_.peek(1))) goto end;
                break;
            case EOF:
                goto end;
            }
            ss << (char)st_.next();
        }
    end:
        return tk_scalar_plain{ss.str()};
    }

    template <class T>
    void roll_indent(int column)
    {
        // check if block sequence starts
        if (indent_ < column) {
            tks_.push_back(T{});
            indents_.push_back(indent_);
            indent_ = column;
        }
    }

    void unroll_indent(int column)
    {
        // check indent and unroll it if needed
        while (indent_ > column) {
            indent_ = indents_.back();
            indents_.pop_back();
            tks_.push_back(tk_block_end{});
        }
    }

    void do_tokenize()
    {
        while (true) {
            st_.skip_ws();

            unroll_indent(st_.column());

            int ch = st_.peek();

            switch (ch) {
            case EOF:
                goto eof;

            case '-':
                if (!is_blankz(st_.peek(1))) break;

                roll_indent<tk_block_sequence_start>(st_.column());
                st_.next();  // skip '-'
                tks_.push_back(tk_block_entry{});

                continue;

            case ':': {
                if (!is_blankz(st_.peek(1))) break;

                auto key = tks_.back();
                tks_.pop_back();

                roll_indent<tk_block_mapping_start>(simple_key_column_);
                st_.next();  // skip ':'
                tks_.push_back(tk_key{});
                tks_.push_back(key);
                tks_.push_back(tk_value{});

                continue;
            }
            }

            simple_key_column_ = st_.column();
            tks_.push_back(fetch_plain_scalar());
        }

    eof:
        unroll_indent(-1);
        tks_.push_back(tk_eof{});
    }

public:
    tokenizer(stream& st) : st_(st), indent_(-1), simple_key_column_(-1)
    {
        do_tokenize();
    }

    const std::vector<token>& get() const
    {
        return tks_;
    }
};

struct unexpected_token {
    token tk;
};

class parser {
private:
    std::vector<token>::iterator tks_iter_;
    value val_;

private:
    template <class T>
    std::optional<T> pop_if()
    {
        auto ptk = std::get_if<T>(&*tks_iter_);
        if (!ptk) return std::nullopt;
        tks_iter_++;
        return *ptk;
    }

    template <class T>
    T expect()
    {
        if (auto tk = pop_if<T>()) return *tk;
        throw unexpected_token();
    }

    template <class T>
    std::optional<T> match()
    {
        auto ptk = std::get_if<T>(&*tks_iter_);
        if (!ptk) return std::nullopt;
        return *ptk;
    }

    value parse_block_collection()
    {
        if (pop_if<tk_block_sequence_start>()) {
            // block_sequnce
            std::vector<value> src;
            while (pop_if<tk_block_entry>())
                src.push_back(parse_block_content());
            expect<tk_block_end>();
            return value(src);
        }

        if (pop_if<tk_block_mapping_start>()) {
            // block_mapping
            std::unordered_map<std::string, value> src;
            while (pop_if<tk_key>()) {
                auto key = expect<tk_scalar_plain>().str;
                expect<tk_value>();
                if (match<tk_block_entry>()) {
                    std::vector<value> lst;
                    while (pop_if<tk_block_entry>())
                        lst.push_back(parse_block_content());
                    src.emplace(key, lst);
                }
                else {
                    auto val = parse_block_content();
                    src.emplace(key, val);
                }
            }
            expect<tk_block_end>();
            return value(src);
        }

        throw unexpected_token();
    }

    value parse_block_content()
    {
        if (auto tk = pop_if<tk_scalar_plain>()) return value(tk->str);
        return parse_block_collection();
    }

    void do_parse()
    {
        val_ = parse_block_content();
    }

public:
    parser(std::vector<token>::iterator tks_iter) : tks_iter_(tks_iter)
    {
        do_parse();
    }

    const value& get()
    {
        return val_;
    }
};

class serializer {
private:
    std::ostream& os_;
    int column_;

private:
    template <class T>
    void println(T&& src)
    {
        os_ << src << std::endl;
        column_ = 0;
    }

    void print_char(char ch)
    {
        os_ << ch;
        if (ch == '\n')
            column_ = 0;
        else
            column_++;
    }

    void print_indent(int indent)
    {
        for (int i = column_; i < indent; i++) print_char(' ');
    }

    void do_serialize(const value& val, int indent)
    {
        val.visit(
            [&](int val) {
                print_indent(indent);
                println(val);
            },
            [&](const std::string& str) {
                print_indent(indent);
                println(str);
            },
            [&](const list& lst) {
                for (auto&& item : lst) {
                    print_indent(indent);
                    print_char('-');
                    print_char(' ');
                    do_serialize(item, indent + 2);
                }
            },
            [&](const map& m) {
                for (auto&& item : m) {
                    print_indent(indent);
                    if (item.second.is_int() || item.second.is_string()) {
                        os_ << item.first << ": ";
                        do_serialize(item.second, 0);
                        continue;
                    }
                    println(item.first + ":");
                    do_serialize(item.second,
                                 item.second.is_list() ? indent : indent + 2);
                }
            });
    }

public:
    serializer(std::ostream& os, const value& val) : os_(os), column_(0)
    {
        do_serialize(val, 0);
    }
};
}  // namespace detail

inline value load(std::istream& is)
{
    detail::stream st = is;
    auto tokens = detail::tokenizer(st).get();
    // for (auto&& tk : tokens)
    //    std::cout << femtoyaml::detail::to_string(tk) << std::endl;
    return detail::parser(tokens.begin()).get();
}

inline value load_string(const std::string& src)
{
    std::stringstream ss(src);
    return load(ss);
}

inline void serialize(std::ostream& os, const value& val)
{
    detail::serializer(os, val);
}

std::string value::to_string() const
{
    std::stringstream ss;
    serialize(ss, *this);
    return ss.str();
}

}  // namespace femtoyaml

#endif

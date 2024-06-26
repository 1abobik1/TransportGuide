#pragma once

#include <string>
#include <variant>
#include <vector>

#include "json.h"

namespace json {

    class Builder final {

        class ItemContext;
        class KeyItemContext;
        class KeyValueItemContext;
        class DictItemContext;
        class ArrayItemContext;

    public:
        Builder() = default;

        // ����������� ���������� std::logic_error ���� �� ������ ������ ������ �����������
        const Node& Build() const;

        KeyItemContext Key(std::string key);
        Builder& Value(NodeData value);

        DictItemContext StartDict();
        Builder& EndDict();

        ArrayItemContext StartArray();
        Builder& EndArray();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        // ��������� �� ��������� ��� �������� �������
        bool is_empty_ = true;
        // ������� ���������� �����
        bool has_key_ = false;
        std::string key_;
        // ��������� ��������� �� ����� ���� � nodes_stack_ � ����������� �� ���� value
        void AddRef(const Node& value);
    };

    // ---- ��������������� ������ ��� �������� ������������ ������� ���������� ----

    class Builder::ItemContext {
    public:
        ItemContext(Builder& builder) : builder_{ builder } {}
    protected:
        KeyItemContext Key(std::string key);
        DictItemContext StartDict();
        Builder& EndDict();
        ArrayItemContext StartArray();
        Builder& EndArray();

        Builder& builder_;
    };

    class Builder::KeyValueItemContext final : public ItemContext {
    public:
        using ItemContext::ItemContext;
        using ItemContext::Key;
        using ItemContext::EndDict;

    };

    class Builder::KeyItemContext final : public ItemContext {
    public:
        using ItemContext::ItemContext;
        KeyValueItemContext Value(NodeData value);
        using ItemContext::StartDict;
        using ItemContext::StartArray;
    };

    class Builder::DictItemContext final : public ItemContext {
    public:
        using ItemContext::ItemContext;
        using ItemContext::Key;
        using ItemContext::EndDict;
    };

    class Builder::ArrayItemContext final : public ItemContext {
    public:
        using ItemContext::ItemContext;
        ArrayItemContext Value(NodeData value);
        using ItemContext::StartDict;
        using ItemContext::StartArray;
        using ItemContext::EndArray;
    };

} // namespace json
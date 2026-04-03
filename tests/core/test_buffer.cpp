#include <gtest/gtest.h>
#include <string>
#include <stdexcept>
#include <string_view>

#include "AudioTaskBuffer.hpp"

using namespace audiotask::core;

//
// AudioTaskBuffer
//
/// @brief 测试空buffer
TEST(AudioTaskBufferTest, DefaultConstructedBufferIsEmpty)
{
    AudioTaskBuffer buffer;

    EXPECT_EQ(buffer.size(), 0u);
    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.str(), std::string_view{});
    EXPECT_EQ(buffer.copy(), std::string{});
}
/// @brief 测试输入为5的buffer
TEST(AudioTaskBufferTest, ConstructFromStringStoresContent)
{
    AudioTaskBuffer buffer(std::string("hello"));

    EXPECT_EQ(buffer.size(), 5u);
    EXPECT_FALSE(buffer.empty());
    EXPECT_EQ(buffer.str(), std::string_view("hello"));
    EXPECT_EQ(buffer.copy(), std::string("hello"));
}

TEST(AudioTaskBufferTest, StringViewConversionWorks)
{
    AudioTaskBuffer buffer(std::string("world"));

    std::string_view view = buffer;

    EXPECT_EQ(view, "world");
}

TEST(AudioTaskBufferTest, AtReturnsCorrectBytes)
{
    AudioTaskBuffer buffer(std::string("abc"));

    EXPECT_EQ(buffer.at(0), static_cast<uint8_t>('a'));
    EXPECT_EQ(buffer.at(1), static_cast<uint8_t>('b'));
    EXPECT_EQ(buffer.at(2), static_cast<uint8_t>('c'));
}

TEST(AudioTaskBufferTest, RemovePrefixByZeroDoesNothing)
{
    AudioTaskBuffer buffer(std::string("hello"));

    buffer.remove_prefix(0);

    EXPECT_EQ(buffer.size(), 5u);
    EXPECT_EQ(buffer.str(), "hello");
    EXPECT_EQ(buffer.copy(), "hello");
}

TEST(AudioTaskBufferTest, RemovePrefixRemovesBeginning)
{
    AudioTaskBuffer buffer(std::string("hello"));

    buffer.remove_prefix(2);

    EXPECT_EQ(buffer.size(), 3u);
    EXPECT_FALSE(buffer.empty());
    EXPECT_EQ(buffer.str(), "llo");
    EXPECT_EQ(buffer.copy(), "llo");
}

TEST(AudioTaskBufferTest, RemovePrefixToWholeSizeMakesEmpty)
{
    AudioTaskBuffer buffer(std::string("hello"));

    buffer.remove_prefix(5);

    EXPECT_EQ(buffer.size(), 0u);
    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.str(), std::string_view{});
    EXPECT_EQ(buffer.copy(), "");
}

TEST(AudioTaskBufferTest, RemovePrefixStepByStep)
{
    AudioTaskBuffer buffer(std::string("abcdef"));

    buffer.remove_prefix(1);
    EXPECT_EQ(buffer.str(), "bcdef");
    EXPECT_EQ(buffer.size(), 5u);

    buffer.remove_prefix(2);
    EXPECT_EQ(buffer.str(), "def");
    EXPECT_EQ(buffer.size(), 3u);

    buffer.remove_prefix(3);
    EXPECT_EQ(buffer.str(), "");
    EXPECT_EQ(buffer.size(), 0u);
    EXPECT_TRUE(buffer.empty());
}

TEST(AudioTaskBufferTest, CopyReturnsIndependentString)
{
    AudioTaskBuffer buffer(std::string("hello"));

    std::string copied = buffer.copy();
    buffer.remove_prefix(2);

    EXPECT_EQ(copied, "hello");
    EXPECT_EQ(buffer.copy(), "llo");
}

TEST(AudioTaskBufferTest, BufferCanContainBinaryLikeDataInStdString)
{
    std::string s;
    s.push_back('A');
    s.push_back('\0');
    s.push_back('B');

    AudioTaskBuffer buffer(std::move(s));

    EXPECT_EQ(buffer.size(), 3u);
    EXPECT_EQ(buffer.at(0), static_cast<uint8_t>('A'));
    EXPECT_EQ(buffer.at(1), static_cast<uint8_t>('\0'));
    EXPECT_EQ(buffer.at(2), static_cast<uint8_t>('B'));

    std::string copied = buffer.copy();
    ASSERT_EQ(copied.size(), 3u);
    EXPECT_EQ(copied[0], 'A');
    EXPECT_EQ(copied[1], '\0');
    EXPECT_EQ(copied[2], 'B');
}

//
// AudioTaskBufferList
//

TEST(AudioTaskBufferListTest, DefaultConstructedListIsEmpty)
{
    AudioTaskBufferList list;

    EXPECT_EQ(list.size(), 0u);
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.concatenate(), "");
}

TEST(AudioTaskBufferListTest, ConstructFromSingleBuffer)
{
    AudioTaskBuffer buffer(std::string("abc"));
    AudioTaskBufferList list(buffer);

    EXPECT_EQ(list.size(), 3u);
    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list.concatenate(), "abc");
}

TEST(AudioTaskBufferListTest, ConstructFromString)
{
    AudioTaskBufferList list(std::string("hello"));

    EXPECT_EQ(list.size(), 5u);
    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list.concatenate(), "hello");
}

TEST(AudioTaskBufferListTest, AppendConstList)
{
    AudioTaskBufferList a(std::string("hello"));
    AudioTaskBufferList b(std::string(" world"));

    a.append(b);

    EXPECT_EQ(a.size(), 11u);
    EXPECT_EQ(a.concatenate(), "hello world");

    // 假设 append(const&) 不会改 b
    EXPECT_EQ(b.size(), 6u);
    EXPECT_EQ(b.concatenate(), " world");
}

TEST(AudioTaskBufferListTest, AppendMoveList)
{
    AudioTaskBufferList a(std::string("foo"));
    AudioTaskBufferList b(std::string("bar"));

    a.append(std::move(b));

    EXPECT_EQ(a.size(), 6u);
    EXPECT_EQ(a.concatenate(), "foobar");
}

TEST(AudioTaskBufferListTest, ConcatenateWithMaxLen)
{
    AudioTaskBufferList a(std::string("abc"));
    AudioTaskBufferList b(std::string("def"));
    a.append(std::move(b));

    EXPECT_EQ(a.concatenate(0), "");
    EXPECT_EQ(a.concatenate(1), "a");
    EXPECT_EQ(a.concatenate(2), "ab");
    EXPECT_EQ(a.concatenate(3), "abc");
    EXPECT_EQ(a.concatenate(4), "abcd");
    EXPECT_EQ(a.concatenate(6), "abcdef");
    EXPECT_EQ(a.concatenate(100), "abcdef");
}

TEST(AudioTaskBufferListTest, FrontReturnsFirstBuffer)
{
    AudioTaskBufferList list(std::string("hello"));

    AudioTaskBuffer first = list.front();

    EXPECT_EQ(first.copy(), "hello");
    EXPECT_EQ(first.size(), 5u);
}

TEST(AudioTaskBufferListTest, PopFrontReturnsAndRemovesFirstBuffer)
{
    AudioTaskBufferList a(std::string("abc"));
    AudioTaskBufferList b(std::string("def"));
    a.append(std::move(b));

    AudioTaskBuffer first = a.pop_front();

    EXPECT_EQ(first.copy(), "abc");
    EXPECT_EQ(a.size(), 3u);
    EXPECT_EQ(a.concatenate(), "def");
}

TEST(AudioTaskBufferListTest, RemovePrefixWithinSingleBuffer)
{
    AudioTaskBufferList list(std::string("hello"));

    list.remove_prefix(2);

    EXPECT_EQ(list.size(), 3u);
    EXPECT_EQ(list.concatenate(), "llo");
}

TEST(AudioTaskBufferListTest, RemovePrefixAcrossMultipleBuffers)
{
    AudioTaskBufferList a(std::string("hello"));
    AudioTaskBufferList b(std::string(" "));
    AudioTaskBufferList c(std::string("world"));

    a.append(std::move(b));
    a.append(std::move(c));

    ASSERT_EQ(a.concatenate(), "hello world");
    ASSERT_EQ(a.size(), 11u);

    a.remove_prefix(6); // remove "hello "

    EXPECT_EQ(a.size(), 5u);
    EXPECT_EQ(a.concatenate(), "world");
}

TEST(AudioTaskBufferListTest, RemovePrefixStepByStepAcrossBuffers)
{
    AudioTaskBufferList a(std::string("ab"));
    AudioTaskBufferList b(std::string("cd"));
    AudioTaskBufferList c(std::string("ef"));

    a.append(std::move(b));
    a.append(std::move(c));

    EXPECT_EQ(a.concatenate(), "abcdef");

    a.remove_prefix(1);
    EXPECT_EQ(a.concatenate(), "bcdef");
    EXPECT_EQ(a.size(), 5u);

    a.remove_prefix(2);
    EXPECT_EQ(a.concatenate(), "def");
    EXPECT_EQ(a.size(), 3u);

    a.remove_prefix(3);
    EXPECT_EQ(a.concatenate(), "");
    EXPECT_EQ(a.size(), 0u);
    EXPECT_TRUE(a.empty());
}

TEST(AudioTaskBufferListTest, BuffersAccessorReflectsUnderlyingCount)
{
    AudioTaskBufferList a(std::string("ab"));
    AudioTaskBufferList b(std::string("cd"));

    a.append(std::move(b));

    EXPECT_GE(a.buffers().size(), 1u);
}

//
// AudioTaskBufferViewList
//

TEST(AudioTaskBufferViewListTest, ConstructFromStdString)
{
    std::string s = "hello";
    AudioTaskBufferViewList views(s);

    EXPECT_EQ(views.size(), 5u);
}

TEST(AudioTaskBufferViewListTest, ConstructFromCString)
{
    AudioTaskBufferViewList views("hello");

    EXPECT_EQ(views.size(), 5u);
}

TEST(AudioTaskBufferViewListTest, ConstructFromStringView)
{
    std::string_view sv = "hello";
    AudioTaskBufferViewList views(sv);

    EXPECT_EQ(views.size(), 5u);
}

TEST(AudioTaskBufferViewListTest, ConstructFromBufferList)
{
    AudioTaskBufferList a(std::string("abc"));
    AudioTaskBufferList b(std::string("def"));
    a.append(std::move(b));

    AudioTaskBufferViewList views(a);

    EXPECT_EQ(views.size(), 6u);
}

TEST(AudioTaskBufferViewListTest, RemovePrefixWithinSingleView)
{
    AudioTaskBufferViewList views(std::string_view("hello"));

    views.remove_prefix(2);

    EXPECT_EQ(views.size(), 3u);
}

TEST(AudioTaskBufferViewListTest, RemovePrefixAcrossMultipleViewsFromBufferList)
{
    AudioTaskBufferList a(std::string("ab"));
    AudioTaskBufferList b(std::string("cd"));
    AudioTaskBufferList c(std::string("ef"));

    a.append(std::move(b));
    a.append(std::move(c));

    AudioTaskBufferViewList views(a);
    EXPECT_EQ(views.size(), 6u);

    views.remove_prefix(1);
    EXPECT_EQ(views.size(), 5u);

    views.remove_prefix(2);
    EXPECT_EQ(views.size(), 3u);

    views.remove_prefix(3);
    EXPECT_EQ(views.size(), 0u);
}

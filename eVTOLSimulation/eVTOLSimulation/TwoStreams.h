#pragma once
/*
 * Name:  TwoStreams.h
 * Description:  Declare a two streams' out simultaneous.
 * Revision:     Date:           Reason                               Author
 * 1.0           Sep 25, 2023    Original                             Chris Wang* 1.3
 *								 Two streams struct delaration
 *
 */
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

// Two streams sturct constructor
struct TwoStreams
{
	TwoStreams(std::ostream& out1, std::ostream& out2) : out1_(out1), out2_(out2) {}

	std::ostream& out1_;
	std::ostream& out2_;
};

// Polymorphism
// Operator overloading, one argument is TwoStreams struct pointer to two outputs, input is a T pointer
// T can be int, float, string value
template <typename T>
TwoStreams& operator<<(TwoStreams& h, T const& t)
{
	h.out1_ << t;
	h.out2_ << t;
	return h;
}

// Operator overloading, one argument is TwoStreams struct pointer to two outputs, input is a ostream pointer
TwoStreams& operator<<(TwoStreams& h, std::ostream& (*f)(std::ostream&))
{
	h.out1_ << f;
	h.out2_ << f;
	return h;
}
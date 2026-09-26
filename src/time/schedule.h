/**
 * @file schedule.h
 *
 * Defines the generation of date schedules for cash flows and interest periods.
 *
 * Responsibilities:
 * - Provide the interface to generate sequences of dates (e.g., coupon payment
 *   dates for a bond or loan).
 * - Combine start date, end date, frequency, Calendar, and Business Day
 * Convention to produce strict, adjusted periods.
 * - Support features like short/long first or last stub periods.
 */

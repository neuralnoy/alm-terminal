/**
 * @file conventions.hpp
 *
 * Defines market conventions for standard financial products.
 *
 * Responsibilities:
 * - Group together standard rules for a specific market or instrument (e.g.,
 * USD Libor Swap).
 * - Combine elements like Day Count Convention, Business Day Convention,
 * Settlement Days, and Calendar into a single unified "Convention" object.
 * - Serve as a central factory or repository for looking up default trading
 * rules across different currencies and instrument types.
 */

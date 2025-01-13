# Sectr Fork

This is a fork of the compiler intended for usage with [SectrPrototype](https://github.com/Ed94/SectrPrototype)

There were additions made for quality of life reasons:

* Added #region, #endregion directives support for editors (I may remove this...)
* I added support for 'monlithic packages' or 'uniform-across-subdirectories packages'. It allows me to organize the main package with sub-directories. (Could be flag instead but making it a file flag works well)
* Added the ability to debug using statements on structs (fields get dumped to the stack as ptr refs)
  * They show up as `struct_name::field_name`
* Remove implicit assignments for container allocators in the Base and Core packages
  * I did not enjoy bug hunting a memory corruption because I mistakenly didn't properly initialize a core container with their designated initiatizer: new, make, or init.
  * Affects the following:
    * base:runtime/core_builtin.doin:`_reserve_dynamic_array`, `_resize_dynamic_array`, `_shrink_dynamic_array`
    * base:runtime/core_builtin_soa.odin:`make_soa_aligned`, `_reserve_soa`
    * base:runtime/default_temp_allocator_arena.odin:`arena_alloc`
    * base:runtime/dynamic_array_internal.odin:`__dynamic_array_reserve`, `__dynamic_array_shrink`
    * base:runtime/dynamic_map_internal.odin:`map_shrink_dynamic`
    * core:flags/internal_assignment.doin:`set_key_value`
    * core:mem/tracking_allocator.odin:`tracking_allocator_proc`
    * core:strings/intern.odin:`_intern_get_entry`

---

<p align="center">
    <img src="misc/logo-slim.png" alt="Odin logo" style="width:65%">
    <br/>
   The Data-Oriented Language for Sane Software Development.
    <br/>
    <br/>
    <a href="https://github.com/odin-lang/odin/releases/latest">
        <img src="https://img.shields.io/github/release/odin-lang/odin.svg">
    </a>
    <a href="https://github.com/odin-lang/odin/releases/latest">
        <img src="https://img.shields.io/badge/platforms-Windows%20|%20Linux%20|%20macOS-green.svg">
    </a>
    <br>
    <a href="https://discord.com/invite/sVBPHEv">
        <img src="https://img.shields.io/discord/568138951836172421?logo=discord">
    </a>
    <a href="https://github.com/odin-lang/odin/actions">
        <img src="https://github.com/odin-lang/odin/workflows/CI/badge.svg?branch=master&event=push">
    </a>
</p>

# The Odin Programming Language


Odin is a general-purpose programming language with distinct typing, built for high performance, modern systems, and built-in data-oriented data types. The Odin Programming Language, the C alternative for the joy of programming.

Website: [https://odin-lang.org/](https://odin-lang.org/)

```odin
package main

import "core:fmt"

main :: proc() {
	program := "+ + * 😃 - /"
	accumulator := 0

	for token in program {
		switch token {
		case '+': accumulator += 1
		case '-': accumulator -= 1
		case '*': accumulator *= 2
		case '/': accumulator /= 2
		case '😃': accumulator *= accumulator
		case: // Ignore everything else
		}
	}

	fmt.printf("The program \"%s\" calculates the value %d\n",
	           program, accumulator)
}

```

## Documentation

#### [Getting Started](https://odin-lang.org/docs/install)

Instructions for downloading and installing the Odin compiler and libraries.

#### [Nightly Builds](https://odin-lang.org/docs/nightly/)

Get the latest nightly builds of Odin.

### Learning Odin

#### [Overview of Odin](https://odin-lang.org/docs/overview)

An overview of the Odin programming language.

#### [Frequently Asked Questions (FAQ)](https://odin-lang.org/docs/faq)

Answers to common questions about Odin.

#### [Packages](https://pkg.odin-lang.org/)

Documentation for all the official packages part of the [core](https://pkg.odin-lang.org/core/) and [vendor](https://pkg.odin-lang.org/vendor/) library collections.

#### [Odin Documentation](https://odin-lang.org/docs/)

Documentation for the Odin language itself.

#### [Odin Discord](https://discord.gg/sVBPHEv)

Get live support and talk with other Odin programmers on the Odin Discord.

### Articles

#### [The Odin Blog](https://odin-lang.org/news/)

The official blog of the Odin programming language, featuring announcements, news, and in-depth articles by the Odin team and guests.

## Warnings

* The Odin compiler is still in development.

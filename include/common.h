/*
This file is part of Sharpscale
Copyright © 2020 浅倉麗子

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef COMMON_H
#define COMMON_H

#define GLZ(x) do {\
	if ((x) < 0) { goto fail; }\
} while (0)

#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

#define UNUSED __attribute__ ((unused))
#define USED __attribute__ ((used))

#endif

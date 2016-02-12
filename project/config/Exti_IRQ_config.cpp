/* Copyright (C) 2015  Nils Weiss
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. */

extern "C" {
extern void MpuInterruptHandler(void);

void EXTI0_IRQHandler(void)
{
    constexpr const Exti& exti = hal::Factory<Exti>::getByExtiLine<EXTI_Line0>();
    exti.handleInterrupt();
}

void EXTI1_IRQHandler(void)
{
    constexpr const Exti& exti = hal::Factory<Exti>::getByExtiLine<EXTI_Line1>();
    exti.handleInterrupt();
}

// TODO Uncomment if you want to use on to the following Extis
void EXTI2_TSC_IRQHandler(void)
{
    //constexpr const Exti& exti = hal::Factory<Exti>::getByExtiLine<EXTI_Line2>();
    //exti.handleInterrupt();
}

void EXTI3_IRQHandler(void)
{
//    constexpr const Exti& exti = hal::Factory<Exti>::getByExtiLine<EXTI_Line3>();
//    exti.handleInterrupt();
}

void EXTI4_IRQHandler(void)
{
//	  constexpr const Exti& exti = hal::Factory<Exti>::getByExtiLine<EXTI_Line4>();
//    exti.handleInterrupt();
}

void EXTI9_5_IRQHandler(void)
{
//    constexpr const Exti& exti5 = hal::Factory<Exti>::getByExtiLine<EXTI_Line5>();
//    exti5.handleInterrupt();
//    constexpr const Exti& exti6 = hal::Factory<Exti>::getByExtiLine<EXTI_Line6>();
//    exti6.handleInterrupt();
//    constexpr const Exti& exti7 = hal::Factory<Exti>::getByExtiLine<EXTI_Line7>();
//    exti7.handleInterrupt();
//    constexpr const Exti& exti8 = hal::Factory<Exti>::getByExtiLine<EXTI_Line8>();
//    exti8.handleInterrupt();
//    constexpr const Exti& exti9 = hal::Factory<Exti>::getByExtiLine<EXTI_Line9>();
//    exti9.handleInterrupt();
}

void EXTI15_10_IRQHandler(void)
{
//    constexpr const Exti& exti10 = hal::Factory<Exti>::getByExtiLine<EXTI_Line10>();
//    exti10.handleInterrupt();
//    constexpr const Exti& exti11 = hal::Factory<Exti>::getByExtiLine<EXTI_Line11>();
//    exti11.handleInterrupt();
//   constexpr const Exti& exti12 = hal::Factory<Exti>::getByExtiLine<EXTI_Line12>();
//    exti12.handleInterrupt();
//    constexpr const Exti& exti13 = hal::Factory<Exti>::getByExtiLine<EXTI_Line13>();
//    exti13.handleInterrupt();
//    constexpr const Exti& exti14 = hal::Factory<Exti>::getByExtiLine<EXTI_Line14>();
//    exti14.handleInterrupt();
//    constexpr const Exti& exti15 = hal::Factory<Exti>::getByExtiLine<EXTI_Line15>();
//    exti15.handleInterrupt();
}
}

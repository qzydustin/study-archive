% There are five houses, each with a different color and inhabited by a house of a different nationality, with a different pet, drink, and favorite TV program.

%     1.The American lives in the red house.
%     2.The Spaniard owns the dog.
%     3.Coffee is drunk in the green house.
%     4.The Ukranian drinks tea.
%     5.The green-house is immediately to the right of the ivory house.
%     6.The house who watches Succession owns snails.
%     7.Westworld is watched in the yellow house.
%     8.Milk is drunk in the middle house.
%     9.The Norwegian lives in the first house on the left.
%     10.The house who watches ER lives in the house next to the house with the fox.
%     11.Westworld is watched in the house next to the house where the horse is kept.
%     12.The house who watches Breaking Bad drinks orange juice.
%     13.The Japanese man watches the Game of Thrones.
%     14.The Norwegian lives next to the blue house.

% Who owns the zebra? Who drinks water? 

% How to run?
% consult('ZebraPuzzle').
% main(Houses).

main(Houses) :-
    Houses = [house(_,_,_,_,_), house(_,_,_,_,_), house(_,_,_,_,_), house(_,_,_,_,_), house(_,_,_,_,_)],
    
    (nationality(Person1,american),color(Person1,red),member(Person1, Houses)),

    (nationality(Person2,spaniard),pet(Person2,dog),member(Person2, Houses)),

    (color(Person3,green),drink(Person3,coffee),member(Person3, Houses)),

    (nationality(Person4,ukranian),drink(Person4,tea),member(Person4, Houses)),

    (right(Person5_1,Person5_2,Houses),color(Person5_1,green),color(Person5_2,ivory),member(Person5_1, Houses),member(Person5_2, Houses)),

    (tv(Person6,succession),pet(Person6,snail),member(Person6, Houses)),

    (tv(Person7,westworld),color(Person7,yellow),member(Person7, Houses)),

    (middle(Person8,Houses),drink(Person8,milk),member(Person8, Houses)),

    (first(Person9,Houses),nationality(Person9,norwegian),member(Person9, Houses)),

    (next(Person10_1,Person10_2,Houses),tv(Person10_1,er),pet(Person10_2,fox),member(Person10_1, Houses),member(Person10_2, Houses)),

    (next(Person11_1,Person11_2,Houses),tv(Person11_1,westworld),pet(Person11_2,horse),member(Person11_1, Houses),member(Person11_2, Houses)),

    (tv(Person12,breaking_bad),drink(Person12,orange_juice),member(Person12, Houses)),

    (nationality(Person13,japanese),tv(Person13,game_of_thrones),member(Person13, Houses)),

    (nationality(Person14_1,norwegian),next(Person14_1,Person14_2,Houses),color(Person14_2,blue),member(Person14_1, Houses),member(Person14_2, Houses)),

    (pet(Person15,zebra),member(Person15,Houses)),

    (drink(Person16,water),member(Person16,Houses)).

color(house(X, _, _, _, _), X).
nationality(house(_, X, _, _, _), X).
pet(house(_, _, X, _, _), X).
drink(house(_, _, _, X, _), X).
tv(house(_, _, _, _, X), X).

right(X,Y,Houses) :- Houses = [Y,X,_,_,_]; Houses = [_,Y,X,_,_]; Houses = [_,_,Y,X,_]; Houses = [_,_,_,Y,X].
next(X,Y,Houses) :- right(X,Y,Houses); right(Y,X,Houses).
middle(X,Houses) :- Houses = [_,_,X,_,_].
first(X,Houses) :- Houses = [X,_,_,_,_].

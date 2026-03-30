A lambda calculus interpreter from scratch in C because why not

Try equations of the form

(\x.x) y // It applies y to the function \x.x and shows the entire beta reduction

example output

```bash
Lambda> (\x.x) y

Parsed: ((\x.x) y)

Reduction:
 -> y

Normal form: y
```

Church Numerals

```bash
Lambda> (\n.\f.\x.f (n f x)) (\f.\x.f (f x))

Parsed: ((\n.(\f.(\x.(f ((n f) x))))) (\f.(\x.(f (f x)))))

Reduction:
 -> (\f.(\x.(f (((\f.(\x.(f (f x)))) f) x))))
 -> (\f.(\x.(f ((\x.(f (f x))) x))))
 -> (\f.(\x.(f (f (f x)))))

Normal form (Church numeral): 3
```

Church Encoded Booleans

```bash
Lambda> (\x.\y.x)

Parsed: (\x.(\y.x))

Reduction:

Normal form: TRUE

Lambda> (\x.\y.y)

Parsed: (\x.(\y.y))

Reduction:

Normal form: FALSE
```

Try the Y combinator yourslef - (\f.(\x.f (x x)) (\x.f (x x))) g

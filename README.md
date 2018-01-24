# Sukantsu
Projet de S4

# DONE List
- Histogramme
- Struct tile **(voir TODO)**

# TODO List
## Mur
- Implémentation: Histogramme
- Piocher: `r = rand() % length`, choisir la r ième valeur

## Main
- Implémentation: Histogramme + Liste de liste (groupes) + valeur (dernière tuile)
- Histogramme: contient les tuiles non groupées
- Liste de groupes: `[[info, el1, el2, el3, el4], ...]`
  - info: 0 si caché, 1 si découvert
  - el1, el2, el3, el4: tuiles (index) du groupe. `défault: -1`
  - Les listes sont statiques (toujours la même taille), les *non-tuiles* sont notées *-1*
- Ajouter des champs si besoin

## Struct Tile
- Il ne semble pas y avoir d'intérêt à la garder (pas d'utilisation pour l'instant, *voir au-dessus*)

## Tout le reste
- voilà, voilà

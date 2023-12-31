
# Train Cohorting

## Milestones

- [ ] Get a train to follow another at a safe distance
- [ ] Cohort stopping together
- [ ] Cohort reversing together
- [ ] Additional trains joining/leaving a cohort

## API

new command: `co <train> <cohort-id>`
- will add a train to a given cohort
- cohort id of zero is invalid
- the train with the same id as the cohort id is the 'master' of the cohort
- `co` command with cohort id also has effect of setting cohort id of train back to itself
- when a train joins a cohort, it will automatically join with the master
- train control on a train that has a cohort id not equal to itself is a NO-OP

train control commands (`tr`, `rv`, `path`) etc on single train in cohort will affect the entire cohort

all trains have slightly different velocities, so each follower train should try to match the leader's velocity as close as possible (even if the speed setting ends up being different).
- this means we would ideally like to gather velocity data for each speed setting
- we can also oscillate between two speed settings to approximate an intermediate velocity

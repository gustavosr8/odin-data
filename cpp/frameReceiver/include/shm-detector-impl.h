struct shm_names {
	char shm_name[NAME_MAX];
	char sem_free_name[NAME_MAX];
	char sem_data_name[NAME_MAX];
	char sem_start_name[NAME_MAX];
};

static int set_shm_names(struct shm_names *names, const char *shm_name)
{
	if (strlen(shm_name) >= NAME_MAX - strlen("/-start"))
		return -1;

	snprintf(names->shm_name, sizeof names->shm_name, "/%s", shm_name);
	snprintf(names->sem_free_name, sizeof names->sem_free_name, "%s-free", names->shm_name);
	snprintf(names->sem_data_name, sizeof names->sem_data_name, "%s-data", names->shm_name);
	snprintf(names->sem_start_name, sizeof names->sem_start_name, "%s-start", names->shm_name);

	return 0;
}

static int open_sem_handles(struct sem_handles *sems, const struct shm_names *names, int create)
{
	int flags = create ? O_CREAT : 0;
	sems->sem_free = sem_open(names->sem_free_name, flags, 0660, 0);
	sems->sem_data = sem_open(names->sem_data_name, flags, 0660, 0);
	sems->sem_start = sem_open(names->sem_start_name, flags, 0660, 0);
	if (!sems->sem_free || !sems->sem_data || !sems->sem_start) {
		perror("sem_open");
		return -1;
	}
	return 0;
}

[[maybe_unused]]
static void close_sem_handles(struct sem_handles *sems)
{
	sem_close(sems->sem_free);
	sem_close(sems->sem_data);
	sem_close(sems->sem_start);
}

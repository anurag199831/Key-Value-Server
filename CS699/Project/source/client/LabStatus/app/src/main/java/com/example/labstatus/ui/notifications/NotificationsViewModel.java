package com.example.labstatus.ui.notifications;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

public class NotificationsViewModel extends ViewModel {

    private MutableLiveData<String> mText;
    public String text;

    public String color;

    public NotificationsViewModel() {
        mText = new MutableLiveData<>();
        mText.setValue("This is Basement fragment");
    }

    public NotificationsViewModel(String t,String c) {

        text=t;
        color=c;
    }

    public LiveData<String> getText() {
        return mText;
    }
}